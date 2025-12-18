#!/usr/bin/env python3
# Simple wrapper that reads UTF-8 text from stdin and outputs jieba segmented tokens separated by spaces
import sys
try:
    import jieba
except Exception as e:
    sys.stderr.write('jieba import error: ' + str(e))
    sys.exit(2)

data = sys.stdin.read()
if not data:
    sys.exit(0)
tokens = jieba.lcut(data)
out = ' '.join(t for t in tokens if t.strip())
sys.stdout.write(out)
