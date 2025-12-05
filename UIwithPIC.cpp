#define UNICODE
#define _UNICODE
#include <windows.h>
#include <unordered_map>
#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <commdlg.h>
#include <sstream>
#include <stdint.h>
#include <thread>
#include <atomic>
#include <mutex>
#include <gdiplus.h>
using namespace Gdiplus;

using namespace std;

// 哈夫曼树相关实现（保持不变）
vector<pair<wchar_t, int>> getFrequencySorted(const wstring& text) {
    unordered_map<wchar_t, int> freqMap;
    for (wchar_t c : text) {
        freqMap[c]++;
    }

    vector<pair<wchar_t, int>> freqVec(freqMap.begin(), freqMap.end());

    sort(freqVec.begin(), freqVec.end(), 
        [](const pair<wchar_t, int>& a, const pair<wchar_t, int>& b) {
            if (a.second != b.second) {
                return a.second < b.second;
            } else {
                return a.first < b.first;
            }
        });

    return freqVec;
}

// 新增：获取字节频率统计（用于图片）
vector<pair<BYTE, int>> getByteFrequencySorted(const vector<BYTE>& data) {
    unordered_map<BYTE, int> freqMap;
    for (BYTE b : data) {
        freqMap[b]++;
    }

    vector<pair<BYTE, int>> freqVec(freqMap.begin(), freqMap.end());

    sort(freqVec.begin(), freqVec.end(), 
        [](const pair<BYTE, int>& a, const pair<BYTE, int>& b) {
            if (a.second != b.second) {
                return a.second < b.second;
            } else {
                return a.first < b.first;
            }
        });

    return freqVec;
}

class HuffmanNode {
public:
    union {
        wchar_t ch;    // 用于文本
        BYTE byte;     // 用于图片
    };
    int freq;
    HuffmanNode* parent;
    HuffmanNode* left;
    HuffmanNode* right;
    bool isByte;     // 标记是字节还是字符

    HuffmanNode(wchar_t character, int frequency) 
        : ch(character), freq(frequency), parent(nullptr), left(nullptr), right(nullptr), isByte(false) {}
    
    HuffmanNode(BYTE b, int frequency) 
        : byte(b), freq(frequency), parent(nullptr), left(nullptr), right(nullptr), isByte(true) {}
};

class HuffmanTree {
private:
    HuffmanNode* root;
    unordered_map<wchar_t, wstring> charToCode;  // 字符到编码的映射（文本）
    unordered_map<wstring, wchar_t> codeToChar;  // 编码到字符的映射（文本）
    unordered_map<BYTE, wstring> byteToCode;     // 字节到编码的映射（图片）
    unordered_map<wstring, BYTE> codeToByte;     // 编码到字节的映射（图片）
    vector<HuffmanNode*> leafnodes;
    bool isImageTree;  // 标记当前树是用于图片还是文本

    void destroyNode(HuffmanNode* node) {
        if (node) {
            destroyNode(node->left);
            destroyNode(node->right);
            delete node;
        }
    }

public:
    HuffmanTree() : root(nullptr), isImageTree(false) {}
    ~HuffmanTree() {
        destroyNode(root);
        root = nullptr;
        leafnodes.clear();
        charToCode.clear();
        codeToChar.clear();
        byteToCode.clear();
        codeToByte.clear();
    }

    // 构建文本哈夫曼树
    void buildForText(const vector<pair<wchar_t, int>>& freqVec) {
        destroyNode(root);
        root = nullptr;
        leafnodes.clear();
        charToCode.clear();
        codeToChar.clear();
        byteToCode.clear();
        codeToByte.clear();
        isImageTree = false;

        vector<HuffmanNode*> nodes;
        for (const auto& p : freqVec) {
            HuffmanNode* newNode = new HuffmanNode(p.first, p.second);
            nodes.push_back(newNode);
            leafnodes.push_back(newNode);
            charToCode[p.first] = L"";
        }

        while (nodes.size() > 1) {
            sort(nodes.begin(), nodes.end(), [](HuffmanNode* a, HuffmanNode* b) {
                if (a->freq != b->freq) {
                    return a->freq < b->freq;
                } else {
                    bool aIsLeaf = (a->left == nullptr && a->right == nullptr);
                    bool bIsLeaf = (b->left == nullptr && b->right == nullptr);
                    if (aIsLeaf && !bIsLeaf) return true;
                    if (!aIsLeaf && bIsLeaf) return false;
                    int aVal = a->isByte ? (int)a->byte : (int)a->ch;
                    int bVal = b->isByte ? (int)b->byte : (int)b->ch;
                    return aVal < bVal;
                }
            });
            HuffmanNode* left = nodes[0];
            HuffmanNode* right = nodes[1];
            HuffmanNode* parent = new HuffmanNode(L'\0', left->freq + right->freq);
            parent->left = left;
            parent->right = right;
            left->parent = parent;
            
            right->parent = parent;
            nodes.erase(nodes.begin());
            nodes.erase(nodes.begin());
            nodes.push_back(parent);
        }
        if (!nodes.empty()) {
            root = nodes[0];
        }

        for (size_t i = 0; i < leafnodes.size(); ++i) {
            HuffmanNode* current = leafnodes[i];
            wstring code = L"";
            while (current->parent != nullptr) {
                if (current == current->parent->left) {
                    code = L"0" + code;
                } else {
                    code = L"1" + code;
                }
                current = current->parent;
            }
            // 文本树：填充字符->编码 和 编码->字符 映射
            wchar_t ch = leafnodes[i]->ch;
            charToCode[ch] = code;
            codeToChar[code] = ch;
        }
    }

    unordered_map<wchar_t, wstring> getCharCodeMap() const {
        return charToCode;
    }

    unordered_map<BYTE, wstring> getByteCodeMap() const {
        return byteToCode;
    }

    wstring decodeText(const wstring& code) const {
        if (codeToChar.empty()) return L"";
        
        wstring result = L"";
        wstring currentCode;
        for (wchar_t bit : code) {
            currentCode += bit;
            auto it = codeToChar.find(currentCode);
            if (it != codeToChar.end()) {
                result += it->second;
                currentCode.clear();
            }
        }
        if (!currentCode.empty()) {
            return L"解码错误：存在无效编码";
        }
        return result;
    }

    // 新增：解码图片数据
    vector<BYTE> decodeImage(const wstring& code) const {
        if (codeToByte.empty()) return {};
        
        vector<BYTE> result;
        wstring currentCode;
        for (wchar_t bit : code) {
            currentCode += bit;
            auto it = codeToByte.find(currentCode);
            if (it != codeToByte.end()) {
                result.push_back(it->second);
                currentCode.clear();
            }
        }
        if (!currentCode.empty()) {
            return {};  // 解码失败
        }
        return result;
    }

    // 序列化编码表（支持文本和图片）
    wstring serializeCodes() const {
        wstringstream ss;
        ss << (isImageTree ? L"IMAGE" : L"TEXT") << L"|";  // 标记类型
        
        if (isImageTree) {
            for (const auto& p : byteToCode) {
                ss << (int)p.first << L"|" << p.second << L"|";
            }
        } else {
            for (const auto& p : charToCode) {
                ss << (int)p.first << L"|" << p.second << L"|";
            }
        }
        return ss.str();
    }

    // 兼容 test7withUI: 仅序列化文本编码表（不带类型前缀）
    wstring serializeTextCodes() const {
        wstringstream ss;
        for (const auto& p : charToCode) {
            ss << (int)p.first << L"|" << p.second << L"|";
        }
        return ss.str();
    }

    // 兼容 test7withUI: 反序列化仅文本编码表（不带类型前缀）
    bool deserializeTextCodes(const wstring& data) {
        charToCode.clear();
        codeToChar.clear();
        wstringstream ss(data);
        wstring part;
        while (getline(ss, part, L'|')) {
            if (part.empty()) continue;
            int charCode;
            try {
                charCode = stoi(part);
            } catch (...) {
                return false;
            }
            if (!getline(ss, part, L'|')) return false;
            wchar_t ch = (wchar_t)charCode;
            charToCode[ch] = part;
            codeToChar[part] = ch;
        }
        return !charToCode.empty();
    }

    // 反序列化编码表
    bool deserializeCodes(const wstring& data) {
        charToCode.clear();
        codeToChar.clear();
        byteToCode.clear();
        codeToByte.clear();
        
        wstringstream ss(data);
        wstring type;
        if (!getline(ss, type, L'|')) return false;
        
        isImageTree = (type == L"IMAGE");
        wstring part;
        
        while (getline(ss, part, L'|')) {
            if (part.empty()) continue;
            int val;
            try {
                val = stoi(part);
            } catch (...) {
                return false;
            }
            if (!getline(ss, part, L'|')) return false;
            
            if (isImageTree) {
                BYTE b = (BYTE)val;
                byteToCode[b] = part;
                codeToByte[part] = b;
            } else {
                wchar_t ch = (wchar_t)val;
                charToCode[ch] = part;
                codeToChar[part] = ch;
            }
        }
        return isImageTree ? !byteToCode.empty() : !charToCode.empty();
    }

    bool isImage() const {
        return isImageTree;
    }

    // 新增：构建图片哈夫曼树（与文本构建类似，但使用 BYTE 作为叶子值）
    void buildForImage(const vector<pair<BYTE, int>>& freqVec) {
        destroyNode(root);
        root = nullptr;
        leafnodes.clear();
        charToCode.clear();
        codeToChar.clear();
        byteToCode.clear();
        codeToByte.clear();
        isImageTree = true;

        vector<HuffmanNode*> nodes;
        for (const auto& p : freqVec) {
            HuffmanNode* newNode = new HuffmanNode(p.first, p.second);
            nodes.push_back(newNode);
            leafnodes.push_back(newNode);
            byteToCode[p.first] = L"";
        }

        while (nodes.size() > 1) {
            sort(nodes.begin(), nodes.end(), [](HuffmanNode* a, HuffmanNode* b) {
                if (a->freq != b->freq) {
                    return a->freq < b->freq;
                } else {
                    bool aIsLeaf = (a->left == nullptr && a->right == nullptr);
                    bool bIsLeaf = (b->left == nullptr && b->right == nullptr);
                    if (aIsLeaf && !bIsLeaf) return true;
                    if (!aIsLeaf && bIsLeaf) return false;
                    int aVal = a->isByte ? (int)a->byte : (int)a->ch;
                    int bVal = b->isByte ? (int)b->byte : (int)b->ch;
                    return aVal < bVal;
                }
            });
            HuffmanNode* left = nodes[0];
            HuffmanNode* right = nodes[1];
            HuffmanNode* parent = new HuffmanNode(L'\0', left->freq + right->freq);
            parent->left = left;
            parent->right = right;
            left->parent = parent;
            right->parent = parent;
            nodes.erase(nodes.begin());
            nodes.erase(nodes.begin());
            nodes.push_back(parent);
        }
        if (!nodes.empty()) {
            root = nodes[0];
        }

        for (size_t i = 0; i < leafnodes.size(); ++i) {
            HuffmanNode* current = leafnodes[i];
            wstring code = L"";
            while (current->parent != nullptr) {
                if (current == current->parent->left) {
                    code = L"0" + code;
                } else {
                    code = L"1" + code;
                }
                current = current->parent;
            }
            byteToCode[leafnodes[i]->byte] = code;
            codeToByte[code] = leafnodes[i]->byte;
        }
    }

    // 新增：直接从位流解码图片（二进制位流，MSB-first 每字节）
    vector<BYTE> decodeImageFromBits(const uint8_t* bytes, uint64_t bitCount) const {
        if (codeToByte.empty()) return {};
        vector<BYTE> result;
        wstring currentCode;
        for (uint64_t i = 0; i < bitCount; ++i) {
            uint64_t byteIndex = i / 8;
            int bitIndex = 7 - (int)(i % 8);
            int bit = (bytes[byteIndex] >> bitIndex) & 1;
            currentCode.push_back(bit ? L'1' : L'0');
            auto it = codeToByte.find(currentCode);
            if (it != codeToByte.end()) {
                result.push_back(it->second);
                currentCode.clear();
            }
        }
        if (!currentCode.empty()) return {};
        return result;
    }
};

wstring encodeText(const wstring& text, const unordered_map<wchar_t, wstring>& codeMap) {
    wstring encoded;
    for (wchar_t c : text) {
        auto it = codeMap.find(c);
        if (it != codeMap.end()) {
            encoded += it->second;
        } else {
            return L"";
        }
    }
    return encoded;
}

// 新增：编码图片字节数据
wstring encodeImage(const vector<BYTE>& data, const unordered_map<BYTE, wstring>& codeMap) {
    wstring encoded;
    for (BYTE b : data) {
        auto it = codeMap.find(b);
        if (it != codeMap.end()) {
            encoded += it->second;
        } else {
            return L"";
        }
    }
    return encoded;
}

// 全局变量
HINSTANCE hInst;
HWND hWndInput, hWndEncoded, hWndDecoded;
HWND hWndFilePath, hWndImagePath;  // 新增图片路径框
HWND hWndImageFilePath; // 用于选择图片的编码/解码文件（右侧独立）
HWND hWndImagePreview;             // 新增图片预览框
// 新增：静态标签句柄以便动态布局
HWND hLblTextTitle, hLblInputLabel, hLblImageTitle, hLblImagePathLabel;
HWND hLblImageFilePathLabel, hLblImagePreviewLabel, hLblEncodedLabel, hLblFilePathLabel, hLblDecodedLabel;
HuffmanTree currentTree;
// 新增：后台图片编码状态和缓存
std::vector<uint8_t> g_imageBits;
std::wstring g_codeTableW;
std::atomic<bool> g_hasImageBinary(false);
std::atomic<bool> g_imageProcessing(false);
std::mutex g_imageMutex;
int g_imgWidth = 0, g_imgHeight = 0, g_imgBpp = 0;
uint64_t g_imageBitCount = 0;
HWND g_mainHwnd = NULL;

#define WM_ENCODE_DONE (WM_APP + 1)

// 函数声明
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
void CreateControls(HWND);
void UpdateLayout(HWND hWnd);
void OnGenerateText(HWND);         // 文本生成编码
void OnGenerateImage(HWND);        // 新增：图片生成编码
void OnSave(HWND);
void OnDecodeText(HWND);
void OnDecodeImage(HWND);
wstring OpenFileDialog(HWND, bool isImage);  // 新增isImage参数
wstring SaveFileDialog(HWND);
vector<BYTE> LoadImageData(const wstring& path, int& width, int& height, int& bitsPerPixel);  // 新增
bool SaveImageData(const wstring& path, const vector<BYTE>& data, int width, int height, int bitsPerPixel);  // 新增

// 主函数（保持不变）
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // 初始化 GDI+（用于加载 PNG/JPEG）
    GdiplusStartupInput gdiInput;
    ULONG_PTR gdiToken = 0;
    GdiplusStartup(&gdiToken, &gdiInput, NULL);
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"HuffmanEncoderClass";

    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, L"窗口注册失败！", L"错误", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    hInst = hInstance;

    HWND hWnd = CreateWindowEx(
        0,
        L"HuffmanEncoderClass",
        L"哈夫曼编码解码器（支持文本和图片）",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 1000, 800,  // 增大窗口尺寸
        NULL, NULL, hInstance, NULL
    );

    if (hWnd == NULL) {
        MessageBox(NULL, L"窗口创建失败！", L"错误", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    g_mainHwnd = hWnd;

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    int ret = (int)msg.wParam;
    // 关闭 GDI+
    GdiplusShutdown(gdiToken);
    return ret;
}

// 窗口过程
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_ENCODE_DONE: {
            // 图片后台编码完成，更新 UI
            std::lock_guard<std::mutex> lk(g_imageMutex);
            if (g_hasImageBinary) {
                SetWindowText(hWndEncoded, L"图片编码完成（已准备好保存）");
            } else {
                SetWindowText(hWndEncoded, L"图片编码失败");
            }
            g_imageProcessing = false;
            return 0;
        }
        case WM_CREATE:
            CreateControls(hWnd);
            DragAcceptFiles(hWnd, TRUE);
            break;

        case WM_SIZE:
            // 窗口大小变化时调整布局，保持左右等宽
            UpdateLayout(hWnd);
            break;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case 1:  // 文本生成按钮
                    OnGenerateText(hWnd);
                    break;
                case 2:  // 保存按钮
                    OnSave(hWnd);
                    break;
                case 3:  // 解码文本按钮
                    OnDecodeText(hWnd);
                    break;
                case 4:  // 浏览文本编码文件按钮
                {
                    wstring path = OpenFileDialog(hWnd, false);
                    if (!path.empty()) {
                        SetWindowText(hWndFilePath, path.c_str());
                    }
                    break;
                }
                case 8:  // 浏览图片编码文件（右侧）
                {
                    wstring path = OpenFileDialog(hWnd, false);
                    if (!path.empty()) {
                        SetWindowText(hWndImageFilePath, path.c_str());
                    }
                    break;
                }
                case 5:  // 浏览图片按钮
                {
                    wstring path = OpenFileDialog(hWnd, true);
                                    if (!path.empty()) {
                                        SetWindowText(hWndImagePath, path.c_str());
                                        // 显示图片预览（只支持 BMP）
                                        HBITMAP hBmp = (HBITMAP)LoadImageW(NULL, path.c_str(), IMAGE_BITMAP, 100, 100, LR_LOADFROMFILE);
                                        if (hBmp) {
                                            // 替换预览位图，删除旧的位图句柄以防泄露
                                            HBITMAP prev = (HBITMAP)SendMessage(hWndImagePreview, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmp);
                                            if (prev) DeleteObject(prev);
                                        }
                                    }
                    break;
                }
                case 6:  // 图片生成编码按钮
                    OnGenerateImage(hWnd);
                    break;
                case 7:  // 图片解码按钮
                    OnDecodeImage(hWnd);
                    break;
            }
            break;

        case WM_DROPFILES: {
            HDROP hDrop = (HDROP)wParam;
            wchar_t filePath[MAX_PATH];
            DragQueryFile(hDrop, 0, filePath, MAX_PATH);
            
            HWND hFocus = GetFocus();
            if (hFocus == hWndInput) {
                wstring ext = filePath;
                transform(ext.begin(), ext.end(), ext.begin(), towlower);
                if (ext.find(L".txt") != wstring::npos) {
                    wifstream file(filePath);
                    if (file.is_open()) {
                        wstring content((istreambuf_iterator<wchar_t>(file)), 
                                        istreambuf_iterator<wchar_t>());
                        SetWindowText(hWndInput, content.c_str());
                        file.close();
                    } else {
                        MessageBox(hWnd, L"无法打开文本文件", L"错误", MB_ICONERROR | MB_OK);
                    }
                } else {
                    MessageBox(hWnd, L"请拖放txt格式的文本文件", L"提示", MB_OK);
                }
            } else {
                SetWindowText(hWndFilePath, filePath);
            }
            DragFinish(hDrop);
            break;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hWnd, msg, wParam, lParam);
    }
    return 0;
}

// 创建控件（新增图片处理区域）
void CreateControls(HWND hWnd) {
    // 我们采用动态布局：控件先创建为最小尺寸，随后调用 UpdateLayout 安排位置
    hLblTextTitle = CreateWindow(L"STATIC", L"===== 文本处理 =====", WS_VISIBLE | WS_CHILD | SS_LEFT,
        0, 0, 1, 1, hWnd, NULL, hInst, NULL);

    hLblInputLabel = CreateWindow(L"STATIC", L"输入要编码的文本:", WS_VISIBLE | WS_CHILD,
        0, 0, 1, 1, hWnd, NULL, hInst, NULL);

    hWndInput = CreateWindow(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE |
        ES_AUTOVSCROLL | WS_VSCROLL, 0, 0, 1, 1, hWnd, NULL, hInst, NULL);

    CreateWindow(L"BUTTON", L"生成文本编码", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        0, 0, 1, 1, hWnd, (HMENU)1, hInst, NULL);

    // 图片处理区域（新增）
    hLblImageTitle = CreateWindow(L"STATIC", L"===== 图片处理 =====", WS_VISIBLE | WS_CHILD | SS_LEFT,
        0, 0, 1, 1, hWnd, NULL, hInst, NULL);

    hLblImagePathLabel = CreateWindow(L"STATIC", L"图片路径:", WS_VISIBLE | WS_CHILD,
        0, 0, 1, 1, hWnd, NULL, hInst, NULL);

    hWndImagePath = CreateWindow(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER,
        0, 0, 1, 1, hWnd, NULL, hInst, NULL);

    CreateWindow(L"BUTTON", L"浏览图片...", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        0, 0, 1, 1, hWnd, (HMENU)5, hInst, NULL);

    CreateWindow(L"BUTTON", L"生成图片编码", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        0, 0, 1, 1, hWnd, (HMENU)6, hInst, NULL);

    // 图片编码/解码文件路径（右侧）
    hLblImageFilePathLabel = CreateWindow(L"STATIC", L"编码文件路径（图片）:", WS_VISIBLE | WS_CHILD,
        0, 0, 1, 1, hWnd, NULL, hInst, NULL);

    hWndImageFilePath = CreateWindow(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER,
        0, 0, 1, 1, hWnd, NULL, hInst, NULL);

    CreateWindow(L"BUTTON", L"浏览编码文件(图片)...", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        0, 0, 1, 1, hWnd, (HMENU)8, hInst, NULL);

    // 图片预览框（新增）
    hLblImagePreviewLabel = CreateWindow(L"STATIC", L"图片预览:", WS_VISIBLE | WS_CHILD,
        0, 0, 1, 1, hWnd, NULL, hInst, NULL);

    hWndImagePreview = CreateWindow(L"STATIC", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | SS_BITMAP,
        0, 0, 1, 1, hWnd, NULL, hInst, NULL);
    SendMessage(hWndImagePreview, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)NULL);

    // 公共区域
    hLblEncodedLabel = CreateWindow(L"STATIC", L"编码结果:", WS_VISIBLE | WS_CHILD,
        0, 0, 1, 1, hWnd, NULL, hInst, NULL);

    hWndEncoded = CreateWindow(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE |
        ES_AUTOVSCROLL | WS_VSCROLL | ES_READONLY, 0, 0, 1, 1, hWnd, NULL, hInst, NULL);

    CreateWindow(L"BUTTON", L"保存编码", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        0, 0, 1, 1, hWnd, (HMENU)2, hInst, NULL);

    hLblFilePathLabel = CreateWindow(L"STATIC", L"编码文件路径:", WS_VISIBLE | WS_CHILD,
        0, 0, 1, 1, hWnd, NULL, hInst, NULL);

    hWndFilePath = CreateWindow(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER,
        0, 0, 1, 1, hWnd, NULL, hInst, NULL);

    CreateWindow(L"BUTTON", L"浏览编码文件...", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        0, 0, 1, 1, hWnd, (HMENU)4, hInst, NULL);

    CreateWindow(L"BUTTON", L"解码文本", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        0, 0, 1, 1, hWnd, (HMENU)3, hInst, NULL);

    // 图片解码按钮（位于图片区域）
    CreateWindow(L"BUTTON", L"解码图片", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
        0, 0, 1, 1, hWnd, (HMENU)7, hInst, NULL);

    hLblDecodedLabel = CreateWindow(L"STATIC", L"解码结果:", WS_VISIBLE | WS_CHILD,
        0, 0, 1, 1, hWnd, NULL, hInst, NULL);

    hWndDecoded = CreateWindow(L"EDIT", L"", WS_VISIBLE | WS_CHILD | WS_BORDER | ES_MULTILINE |
        ES_AUTOVSCROLL | WS_VSCROLL | ES_READONLY, 0, 0, 1, 1, hWnd, NULL, hInst, NULL);

    // 初始布局
    // UpdateLayout 会在 WM_CREATE 后和 WM_SIZE 中被调用以保证左右等宽
    UpdateLayout(hWnd);
}

// 动态布局：左右两栏等宽，控件按列垂直排列
void UpdateLayout(HWND hWnd) {
    RECT rc;
    GetClientRect(hWnd, &rc);
    int padding = 12;
    int totalWidth = rc.right - rc.left - padding * 3; // 两边与中间间隙
    if (totalWidth < 200) totalWidth = 200;
    int colW = totalWidth / 2;
    int leftX = padding;
    int rightX = leftX + colW + padding;

    // 统一尺寸
    int labelH = 20;
    int btnH = 30;
    int smallH = 24;

    // 计算列高度可用空间
    int clientH = rc.bottom - rc.top;
    int top = padding;

    // 左列：文本处理
    if (hLblTextTitle) MoveWindow(hLblTextTitle, leftX, top, colW, labelH, TRUE);
    top += labelH + 6;
    if (hLblInputLabel) MoveWindow(hLblInputLabel, leftX, top, 160, labelH, TRUE);
    top += labelH + 4;
    int inputH = max(100, clientH / 5);
    if (hWndInput) MoveWindow(hWndInput, leftX, top, colW, inputH, TRUE);
    top += inputH + 8;
    if (GetDlgItem(hWnd, 1)) MoveWindow(GetDlgItem(hWnd, 1), leftX, top, 140, btnH, TRUE); // 生成文本
    top += btnH + 10;

    if (hLblEncodedLabel) MoveWindow(hLblEncodedLabel, leftX, top, colW, labelH, TRUE);
    top += labelH + 4;
    int encodedH = max(100, clientH / 7);
    if (hWndEncoded) MoveWindow(hWndEncoded, leftX, top, colW, encodedH, TRUE);
    top += encodedH + 8;

    // 保存按钮 + 文件路径 + 浏览
    if (GetDlgItem(hWnd, 2)) MoveWindow(GetDlgItem(hWnd, 2), leftX, top, 100, btnH, TRUE); // 保存编码
    if (hLblFilePathLabel) MoveWindow(hLblFilePathLabel, leftX + 110, top + 4, 100, labelH, TRUE);
    if (hWndFilePath) MoveWindow(hWndFilePath, leftX + 110, top + btnH + 2, colW - 200, smallH, TRUE);
    if (GetDlgItem(hWnd, 4)) MoveWindow(GetDlgItem(hWnd, 4), leftX + colW - 80, top + btnH + 2, 70, smallH, TRUE);
    // 解码文本按钮
    if (GetDlgItem(hWnd, 3)) MoveWindow(GetDlgItem(hWnd, 3), leftX, top + btnH + 2, 120, btnH, TRUE);
    top = top + btnH + 2 + smallH + 12;

    if (hLblDecodedLabel) MoveWindow(hLblDecodedLabel, leftX, top, colW, labelH, TRUE);
    top += labelH + 4;
    int decodedH = max(140, clientH - top - padding);
    if (hWndDecoded) MoveWindow(hWndDecoded, leftX, top, colW, decodedH, TRUE);

    // 右列：图片处理
    int rtop = padding;
    if (hLblImageTitle) MoveWindow(hLblImageTitle, rightX, rtop, colW, labelH, TRUE);
    rtop += labelH + 6;
    if (hLblImagePathLabel) MoveWindow(hLblImagePathLabel, rightX, rtop, 100, labelH, TRUE);
    rtop += labelH + 4;
    if (hWndImagePath) MoveWindow(hWndImagePath, rightX, rtop, colW - 90, smallH, TRUE);
    if (GetDlgItem(hWnd, 5)) MoveWindow(GetDlgItem(hWnd, 5), rightX + colW - 80, rtop, 70, smallH, TRUE);
    rtop += smallH + 8;
    // 生成/解码按钮一行
    if (GetDlgItem(hWnd, 6)) MoveWindow(GetDlgItem(hWnd, 6), rightX, rtop, 140, btnH, TRUE);
    if (GetDlgItem(hWnd, 7)) MoveWindow(GetDlgItem(hWnd, 7), rightX + 150, rtop, 120, btnH, TRUE);
    rtop += btnH + 10;

    if (hLblImageFilePathLabel) MoveWindow(hLblImageFilePathLabel, rightX, rtop, colW, labelH, TRUE);
    rtop += labelH + 4;
    if (hWndImageFilePath) MoveWindow(hWndImageFilePath, rightX, rtop, colW - 120, smallH, TRUE);
    if (GetDlgItem(hWnd, 8)) MoveWindow(GetDlgItem(hWnd, 8), rightX + colW - 110, rtop, 110, smallH, TRUE);
    rtop += smallH + 10;

    if (hLblImagePreviewLabel) MoveWindow(hLblImagePreviewLabel, rightX, rtop, colW, labelH, TRUE);
    rtop += labelH + 6;
    int previewH = max(160, clientH - rtop - padding);
    if (hWndImagePreview) MoveWindow(hWndImagePreview, rightX, rtop, colW, previewH, TRUE);
}

// 生成文本编码（保持不变）
void OnGenerateText(HWND hWnd) {
    int len = GetWindowTextLength(hWndInput) + 1;
    wchar_t* textBuf = new wchar_t[len];
    GetWindowText(hWndInput, textBuf, len);
    wstring text(textBuf);
    delete[] textBuf;
    
    if (text.empty()) {
        MessageBox(hWnd, L"请输入要编码的文本", L"提示", MB_OK);
        return;
    }
    
    auto sortedFreq = getFrequencySorted(text);
    currentTree.buildForText(sortedFreq);
    wstring encoded = encodeText(text, currentTree.getCharCodeMap());
    
    if (encoded.empty()) {
        MessageBox(hWnd, L"编码失败，包含未处理的字符", L"错误", MB_ICONERROR | MB_OK);
        return;
    }
    
    SetWindowText(hWndEncoded, encoded.c_str());
    // Ensure image cache is cleared for text flow
    {
        std::lock_guard<std::mutex> lk(g_imageMutex);
        g_hasImageBinary = false;
    }
}

// 新增：生成图片编码
void OnGenerateImage(HWND hWnd) {
    wchar_t filePath[MAX_PATH];
    GetWindowText(hWndImagePath, filePath, MAX_PATH);
    
    if (wcslen(filePath) == 0) {
        MessageBox(hWnd, L"请选择图片文件", L"提示", MB_OK);
        return;
    }
    
    int width, height, bitsPerPixel;
    vector<BYTE> imageData = LoadImageData(wstring(filePath), width, height, bitsPerPixel);
    
    if (imageData.empty()) {
        MessageBox(hWnd, L"无法读取图片数据", L"错误", MB_ICONERROR | MB_OK);
        return;
    }
    
    // 如果已经在后台处理，提示并返回
    if (g_imageProcessing) {
        MessageBox(hWnd, L"图片正在后台处理，请稍候", L"提示", MB_OK);
        return;
    }

    g_imageProcessing = true;
    SetWindowText(hWndEncoded, L"图片编码已开始（后台处理），请稍候...");

    // 后台线程执行哈夫曼构造与位打包，完成后通过消息通知主线程
    std::thread worker([imageData, width, height, bitsPerPixel]() {
        auto sortedFreq = getByteFrequencySorted(imageData);
        HuffmanTree localTree;
        localTree.buildForImage(sortedFreq);
        auto codeMap = localTree.getByteCodeMap();

        std::vector<uint8_t> localBits;
        localBits.reserve(imageData.size() / 8 + 16);
        uint8_t curByte = 0;
        int bitPos = 7;
        uint64_t totalBits = 0;

        for (BYTE b : imageData) {
            auto it = codeMap.find(b);
            if (it == codeMap.end()) {
                // error
                std::lock_guard<std::mutex> lk(g_imageMutex);
                g_hasImageBinary = false;
                g_imageProcessing = false;
                if (g_mainHwnd) PostMessage(g_mainHwnd, WM_ENCODE_DONE, 0, 0);
                return;
            }
            const wstring& code = it->second;
            for (wchar_t wc : code) {
                int bit = (wc == L'1') ? 1 : 0;
                if (bit) curByte |= (1 << bitPos);
                --bitPos;
                ++totalBits;
                if (bitPos < 0) {
                    localBits.push_back(curByte);
                    curByte = 0;
                    bitPos = 7;
                }
            }
        }
        if (bitPos != 7) {
            localBits.push_back(curByte);
        }

        // serialize code table
        wstring codeTable = localTree.serializeCodes();

        // store into globals
        {
            std::lock_guard<std::mutex> lk(g_imageMutex);
            g_imageBits.swap(localBits);
            g_codeTableW = codeTable;
            g_hasImageBinary = true;
            g_imageBitCount = totalBits;
            g_imgWidth = width;
            g_imgHeight = height;
            g_imgBpp = bitsPerPixel;
        }

        if (g_mainHwnd) PostMessage(g_mainHwnd, WM_ENCODE_DONE, 0, 0);
    });
    worker.detach();
}

// 保存编码（修改为支持图片信息）
void OnSave(HWND hWnd) {
    int len = GetWindowTextLength(hWndEncoded) + 1;
    wchar_t* encodedBuf = new wchar_t[len];
    GetWindowText(hWndEncoded, encodedBuf, len);
    wstring encoded(encodedBuf);
    delete[] encodedBuf;
    
    if (encoded.empty()) {
        MessageBox(hWnd, L"没有可保存的编码内容", L"提示", MB_OK);
        return;
    }
    
    wstring path = SaveFileDialog(hWnd);
    if (path.empty()) return;

    // 如果存在图片二进制缓存，写入二进制格式：CODE_TABLE_START\n<utf8 table>\nCODE_DATA_START\nBIN_IMAGE|w|h|bpp|bitcount\n<raw bytes>
    if (g_hasImageBinary) {
        // convert path to UTF-8
        int need = WideCharToMultiByte(CP_UTF8, 0, path.c_str(), -1, NULL, 0, NULL, NULL);
        std::string pathUtf8(need, 0);
        WideCharToMultiByte(CP_UTF8, 0, path.c_str(), -1, &pathUtf8[0], need, NULL, NULL);

        std::ofstream fout(pathUtf8.c_str(), std::ios::binary);
        if (!fout.is_open()) {
            MessageBox(hWnd, L"保存失败（无法打开文件）", L"错误", MB_ICONERROR | MB_OK);
            return;
        }

        std::wstring codeTableW;
        std::vector<uint8_t> bitsCopy;
        int w=0,h=0,bpp=0;
        uint64_t bitCount=0;
        {
            std::lock_guard<std::mutex> lk(g_imageMutex);
            codeTableW = g_codeTableW;
            bitsCopy = g_imageBits;
            w = g_imgWidth; h = g_imgHeight; bpp = g_imgBpp; bitCount = g_imageBitCount;
        }

        // convert codeTable to UTF-8
        std::string codeTableUtf8;
        if (!codeTableW.empty()) {
            int sz = WideCharToMultiByte(CP_UTF8, 0, codeTableW.c_str(), (int)codeTableW.size(), NULL, 0, NULL, NULL);
            codeTableUtf8.resize(sz);
            WideCharToMultiByte(CP_UTF8, 0, codeTableW.c_str(), (int)codeTableW.size(), &codeTableUtf8[0], sz, NULL, NULL);
        }

        fout << "CODE_TABLE_START\n";
        fout << codeTableUtf8 << "\n";
        fout << "CODE_DATA_START\n";
        fout << "BIN_IMAGE|" << w << "|" << h << "|" << bpp << "|" << bitCount << "\n";
        if (!bitsCopy.empty()) {
            fout.write(reinterpret_cast<const char*>(bitsCopy.data()), bitsCopy.size());
        }
        fout.close();
        MessageBox(hWnd, L"保存成功（已写入二进制位流）", L"提示", MB_OK);
        return;
    }

    // 旧文本保存逻辑（与 test7withUI 格式兼容）
    wofstream file(path.c_str());
    if (file.is_open()) {
        wstring codeTable = currentTree.serializeTextCodes();
        file << L"CODE_TABLE_START" << endl;
        file << codeTable << endl;
        file << L"CODE_DATA_START" << endl;
        file << encoded << endl;
        file.close();
        MessageBox(hWnd, L"保存成功", L"提示", MB_OK);
    } else {
        MessageBox(hWnd, L"保存失败", L"错误", MB_ICONERROR | MB_OK);
    }
}

// 解码（修改为支持图片解码）
// 文本解码（保留原始逻辑，基于宽字符输入/输出）
void OnDecodeText(HWND hWnd) {
    wchar_t filePath[MAX_PATH];
    // 文本解码应从通用编码文件路径控件读取（左侧文本的 hWndFilePath）
    GetWindowText(hWndFilePath, filePath, MAX_PATH);

    if (wcslen(filePath) == 0) {
        MessageBox(hWnd, L"请选择编码文件", L"提示", MB_OK);
        return;
    }

    wifstream file(filePath);
    if (!file.is_open()) {
        MessageBox(hWnd, L"无法打开文件", L"错误", MB_ICONERROR | MB_OK);
        return;
    }

    // 读取编码表和编码数据（保留原来的文本解析方式）
    wstring line;
    wstring codeTable;
    wstring encodedData;
    enum { READING_HEADER, READING_TABLE, READING_DATA } state = READING_HEADER;

    while (getline(file, line)) {
        if (state == READING_HEADER) {
            if (line == L"CODE_TABLE_START") {
                state = READING_TABLE;
            }
        } else if (state == READING_TABLE) {
            if (line == L"CODE_DATA_START") {
                state = READING_DATA;
            } else {
                codeTable += line;
            }
        } else if (state == READING_DATA) {
            encodedData += line;
        }
    }
    file.close();

    if (codeTable.empty() || encodedData.empty()) {
        MessageBox(hWnd, L"无效的编码文件", L"错误", MB_ICONERROR | MB_OK);
        return;
    }

    // 使用与 test7withUI 兼容的文本反序列化
    if (currentTree.deserializeTextCodes(codeTable)) {
        wstring decoded = currentTree.decodeText(encodedData);
        SetWindowText(hWndDecoded, decoded.c_str());
    } else {
        MessageBox(hWnd, L"解码失败，编码表无效", L"错误", MB_ICONERROR | MB_OK);
    }
}

// 图片解码（处理二进制位流或文本图片编码，保存为 BMP）
void OnDecodeImage(HWND hWnd) {
    wchar_t filePath[MAX_PATH];
    // 优先使用右侧图片专属编码文件路径控件（hWndImageFilePath），否则回退到通用路径控件（hWndFilePath）
    GetWindowText(hWndImageFilePath, filePath, MAX_PATH);
    if (wcslen(filePath) == 0) {
        GetWindowText(hWndFilePath, filePath, MAX_PATH);
    }

    if (wcslen(filePath) == 0) {
        MessageBox(hWnd, L"请选择编码文件", L"提示", MB_OK);
        return;
    }

    // 读取为二进制，解析 CODE_TABLE_START / CODE_DATA_START
    int need = WideCharToMultiByte(CP_UTF8, 0, filePath, -1, NULL, 0, NULL, NULL);
    std::string pathUtf8(need, 0);
    WideCharToMultiByte(CP_UTF8, 0, filePath, -1, &pathUtf8[0], need, NULL, NULL);

    std::ifstream fin(pathUtf8.c_str(), std::ios::binary);
    if (!fin.is_open()) {
        MessageBox(hWnd, L"无法打开文件", L"错误", MB_ICONERROR | MB_OK);
        return;
    }

    std::string content((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());
    fin.close();

    const std::string startTag = "CODE_TABLE_START\n";
    const std::string dataTag = "CODE_DATA_START\n";
    size_t pos = content.find(startTag);
    if (pos == std::string::npos) {
        MessageBox(hWnd, L"无效的编码文件（缺少表头）", L"错误", MB_ICONERROR | MB_OK);
        return;
    }
    size_t posData = content.find(dataTag, pos + startTag.size());
    if (posData == std::string::npos) {
        MessageBox(hWnd, L"无效的编码文件（缺少数据标记）", L"错误", MB_ICONERROR | MB_OK);
        return;
    }

    std::string codeTableUtf8 = content.substr(pos + startTag.size(), posData - (pos + startTag.size()));
    while (!codeTableUtf8.empty() && (codeTableUtf8.back() == '\n' || codeTableUtf8.back() == '\r')) codeTableUtf8.pop_back();

    size_t dataStart = posData + dataTag.size();
    size_t newlinePos = content.find('\n', dataStart);
    if (newlinePos == std::string::npos) {
        MessageBox(hWnd, L"无效的数据部分", L"错误", MB_ICONERROR | MB_OK);
        return;
    }
    std::string headerLine = content.substr(dataStart, newlinePos - dataStart);

    // convert code table utf8 -> wstring
    std::wstring codeTableW;
    if (!codeTableUtf8.empty()) {
        int sz = MultiByteToWideChar(CP_UTF8, 0, codeTableUtf8.c_str(), (int)codeTableUtf8.size(), NULL, 0);
        codeTableW.resize(sz);
        MultiByteToWideChar(CP_UTF8, 0, codeTableUtf8.c_str(), (int)codeTableUtf8.size(), &codeTableW[0], sz);
    }

    if (headerLine.rfind("BIN_IMAGE|", 0) == 0) {
        int width=0, height=0, bpp=0;
        uint64_t bitCount = 0;
        {
            std::stringstream ss(headerLine);
            std::string token;
            getline(ss, token, '|'); // BIN_IMAGE
            getline(ss, token, '|'); width = stoi(token);
            getline(ss, token, '|'); height = stoi(token);
            getline(ss, token, '|'); bpp = stoi(token);
            getline(ss, token, '|'); bitCount = stoull(token);
        }

        size_t binStart = newlinePos + 1;
        if (binStart > content.size()) {
            MessageBox(hWnd, L"无效的二进制数据", L"错误", MB_ICONERROR | MB_OK);
            return;
        }
        const unsigned char* binPtr = reinterpret_cast<const unsigned char*>(content.data() + binStart);

        if (!currentTree.deserializeCodes(codeTableW)) {
            MessageBox(hWnd, L"解码失败，编码表无效", L"错误", MB_ICONERROR | MB_OK);
            return;
        }

        vector<BYTE> imageData = currentTree.decodeImageFromBits(binPtr, bitCount);
        if (imageData.empty()) {
            MessageBox(hWnd, L"图片解码失败", L"错误", MB_ICONERROR | MB_OK);
            return;
        }

        wstring savePath = SaveFileDialog(hWnd);
        if (!savePath.empty()) {
            std::wstring sp = savePath;
            std::wstring lower = sp;
            std::transform(lower.begin(), lower.end(), lower.begin(), towlower);
            if (lower.size() < 4 || lower.substr(lower.size()-4) != L".bmp") {
                sp += L".bmp";
            }
            if (SaveImageData(sp, imageData, width, height, bpp)) {
                SetWindowText(hWndDecoded, L"图片解码成功，已保存到指定路径");
                HBITMAP hBmp = (HBITMAP)LoadImageW(NULL, sp.c_str(), IMAGE_BITMAP, 100, 100, LR_LOADFROMFILE);
                if (hBmp) {
                    HBITMAP prev = (HBITMAP)SendMessage(hWndImagePreview, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmp);
                    if (prev) DeleteObject(prev);
                }
            } else {
                SetWindowText(hWndDecoded, L"图片解码成功，但保存失败");
            }
        }
        return;
    }

    // 不是二进制位流，则可能为文本编码格式（兼容旧格式），但图片编码在文本模式下也可能存在
    string encodedUtf8 = content.substr(dataStart);
    std::wstring encodedW;
    if (!encodedUtf8.empty()) {
        int sz = MultiByteToWideChar(CP_UTF8, 0, encodedUtf8.c_str(), (int)encodedUtf8.size(), NULL, 0);
        encodedW.resize(sz);
        MultiByteToWideChar(CP_UTF8, 0, encodedUtf8.c_str(), (int)encodedUtf8.size(), &encodedW[0], sz);
    }

    if (codeTableW.empty() || encodedW.empty()) {
        MessageBox(hWnd, L"无效的编码文件", L"错误", MB_ICONERROR | MB_OK);
        return;
    }

    if (!currentTree.deserializeCodes(codeTableW)) {
        MessageBox(hWnd, L"解码失败，编码表无效", L"错误", MB_ICONERROR | MB_OK);
        return;
    }

    if (currentTree.isImage()) {
        // 文本格式的图片编码，格式为 IMAGE_INFO|w|h|bpp|<bits>
        wstringstream ss(encodedW);
        wstring infoPart;
        if (!getline(ss, infoPart, L'|') || infoPart != L"IMAGE_INFO") {
            MessageBox(hWnd, L"无效的图片编码数据", L"错误", MB_ICONERROR | MB_OK);
            return;
        }
        int width, height, bitsPerPixel;
        wstring val;
        getline(ss, val, L'|'); width = stoi(val);
        getline(ss, val, L'|'); height = stoi(val);
        getline(ss, val, L'|'); bitsPerPixel = stoi(val);
        wstring imageCode;
        getline(ss, imageCode);
        vector<BYTE> imageData = currentTree.decodeImage(imageCode);
        if (imageData.empty()) {
            MessageBox(hWnd, L"图片解码失败", L"错误", MB_ICONERROR | MB_OK);
            return;
        }
        wstring savePath = SaveFileDialog(hWnd);
        if (!savePath.empty()) {
            std::wstring sp = savePath;
            std::wstring lower = sp;
            std::transform(lower.begin(), lower.end(), lower.begin(), towlower);
            if (lower.size() < 4 || lower.substr(lower.size()-4) != L".bmp") {
                sp += L".bmp";
            }
            if (SaveImageData(sp, imageData, width, height, bitsPerPixel)) {
                SetWindowText(hWndDecoded, L"图片解码成功，已保存到指定路径");
                HBITMAP hBmp = (HBITMAP)LoadImageW(NULL, sp.c_str(), IMAGE_BITMAP, 100, 100, LR_LOADFROMFILE);
                if (hBmp) {
                    HBITMAP prev = (HBITMAP)SendMessage(hWndImagePreview, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBmp);
                    if (prev) DeleteObject(prev);
                }
            } else {
                SetWindowText(hWndDecoded, L"图片解码成功，但保存失败");
            }
        }
    } else {
        // 不是图片：按文本解码并显示
        wstring decoded = currentTree.decodeText(encodedW);
        SetWindowText(hWndDecoded, decoded.c_str());
    }
}

// 打开文件对话框（修改为支持图片格式）
wstring OpenFileDialog(HWND hWnd, bool isImage) {
    OPENFILENAME ofn;
    wchar_t szFile[MAX_PATH] = L"";
    
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);
    
    if (isImage) {
        // 支持 BMP/PNG/JPEG
        ofn.lpstrFilter = L"Image Files (*.bmp;*.png;*.jpg;*.jpeg)\0*.bmp;*.png;*.jpg;*.jpeg\0All Files (*.*)\0*.*\0";
    } else {
        ofn.lpstrFilter = L"Huffman Encoded Files (*.huf)\0*.huf\0All Files (*.*)\0*.*\0";
    }
    
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    
    if (GetOpenFileName(&ofn) == TRUE) {
        return wstring(ofn.lpstrFile);
    }
    return L"";
}

// 保存文件对话框（保持不变）
wstring SaveFileDialog(HWND hWnd) {
    OPENFILENAME ofn;
    wchar_t szFile[MAX_PATH] = L"encoded.huf";
    
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);
    ofn.lpstrFilter = L"Huffman Encoded Files (*.huf)\0*.huf\0Bitmap Images (*.bmp)\0*.bmp\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
    
    if (GetSaveFileName(&ofn) == TRUE) {
        return wstring(ofn.lpstrFile);
    }
    return L"";
}

// 新增：加载图片数据
vector<BYTE> LoadImageData(const wstring& path, int& width, int& height, int& bitsPerPixel) {
    // 先尝试按原生 BMP 读取（未压缩 BI_RGB），失败则尝试用 GDI+ 加载 PNG/JPEG
    vector<BYTE> data;
    bool bmpOk = false;

    // 尝试 BMP 读取
    HANDLE hFile = CreateFileW(path.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        DWORD bytesRead = 0;
        BITMAPFILEHEADER bfh;
        if (ReadFile(hFile, &bfh, sizeof(bfh), &bytesRead, NULL) && bytesRead == sizeof(bfh) && bfh.bfType == 0x4D42) {
            BITMAPINFOHEADER bih;
            if (ReadFile(hFile, &bih, sizeof(bih), &bytesRead, NULL) && bytesRead == sizeof(bih)) {
                if (bih.biCompression == BI_RGB) {
                    int w = bih.biWidth;
                    int h = abs(bih.biHeight);
                    int bpp = bih.biBitCount;
                    int bytesPerPixel = bpp / 8;
                    int rowSize = ((bpp * w + 31) / 32) * 4;
                    vector<BYTE> buf(w * h * bytesPerPixel);

                    // seek to pixel data
                    SetFilePointer(hFile, bfh.bfOffBits, NULL, FILE_BEGIN);
                    vector<BYTE> rowBuf(rowSize);
                    bool ok = true;
                    for (int y = 0; y < h; ++y) {
                        if (!ReadFile(hFile, rowBuf.data(), rowSize, &bytesRead, NULL) || bytesRead != (DWORD)rowSize) {
                            ok = false; break;
                        }
                        int dstY = (bih.biHeight > 0) ? (h - 1 - y) : y; // bottom-up if biHeight>0
                        for (int x = 0; x < w; ++x) {
                            for (int b = 0; b < bytesPerPixel; ++b) {
                                buf[(dstY * w + x) * bytesPerPixel + b] = rowBuf[x * bytesPerPixel + b];
                            }
                        }
                    }
                    if (ok) {
                        data.swap(buf);
                        width = w; height = h; bitsPerPixel = bpp;
                        bmpOk = true;
                    }
                }
            }
        }
        CloseHandle(hFile);
    }

    if (bmpOk) return data;

    // BMP 读取失败，尝试使用 GDI+ 加载（支持 PNG/JPEG 等）
    {
        Bitmap* bmp = new Bitmap(path.c_str());
        if (bmp && bmp->GetLastStatus() == Ok) {
            int w = bmp->GetWidth();
            int h = bmp->GetHeight();
            PixelFormat pf = bmp->GetPixelFormat();
            int bpp = GetPixelFormatSize(pf);
            int bytesPerPixel = bpp / 8;
            if (bytesPerPixel < 3) { delete bmp; return {}; }

            Rect rect(0, 0, w, h);
            BitmapData bd;
            if (bmp->LockBits(&rect, ImageLockModeRead, pf, &bd) == Ok) {
                BYTE* src = (BYTE*)bd.Scan0;
                int stride = bd.Stride;
                data.assign(w * h * bytesPerPixel, 0);
                for (int y = 0; y < h; ++y) {
                    BYTE* row = src + y * stride;
                    for (int x = 0; x < w; ++x) {
                        for (int b = 0; b < bytesPerPixel; ++b) {
                            data[(y * w + x) * bytesPerPixel + b] = row[x * bytesPerPixel + b];
                        }
                    }
                }
                bmp->UnlockBits(&bd);
                width = w; height = h; bitsPerPixel = bpp;
                delete bmp;
                return data;
            }
            delete bmp;
        }
    }

    return {};
}

// 新增：保存图片数据
bool SaveImageData(const wstring& path, const vector<BYTE>& data, int width, int height, int bitsPerPixel) {
    // 仅支持写入未压缩 BMP (BI_RGB)
    int bytesPerPixel = bitsPerPixel / 8;
    int rowSize = ((bitsPerPixel * width + 31) / 32) * 4;
    int pixelDataSize = rowSize * height;

    BITMAPFILEHEADER bfh;
    BITMAPINFOHEADER bih;
    ZeroMemory(&bfh, sizeof(bfh));
    ZeroMemory(&bih, sizeof(bih));

    bfh.bfType = 0x4D42; // 'BM'
    bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
    bfh.bfSize = bfh.bfOffBits + pixelDataSize;

    bih.biSize = sizeof(BITMAPINFOHEADER);
    bih.biWidth = width;
    bih.biHeight = height; // bottom-up
    bih.biPlanes = 1;
    bih.biBitCount = (WORD)bitsPerPixel;
    bih.biCompression = BI_RGB;
    bih.biSizeImage = pixelDataSize;

    HANDLE hFile = CreateFileW(path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) return false;

    DWORD written = 0;
    if (!WriteFile(hFile, &bfh, sizeof(bfh), &written, NULL) || written != sizeof(bfh)) { CloseHandle(hFile); return false; }
    if (!WriteFile(hFile, &bih, sizeof(bih), &written, NULL) || written != sizeof(bih)) { CloseHandle(hFile); return false; }

    vector<BYTE> rowBuf(rowSize);
    for (int y = 0; y < height; ++y) {
        // write bottom-up
        int srcY = height - 1 - y;
        memset(rowBuf.data(), 0, rowSize);
        for (int x = 0; x < width; ++x) {
            for (int b = 0; b < bytesPerPixel; ++b) {
                int srcIdx = (srcY * width + x) * bytesPerPixel + b;
                if (srcIdx < (int)data.size()) rowBuf[x * bytesPerPixel + b] = data[srcIdx];
            }
        }
        if (!WriteFile(hFile, rowBuf.data(), rowSize, &written, NULL) || written != (DWORD)rowSize) { CloseHandle(hFile); return false; }
    }

    CloseHandle(hFile);
    return true;
}