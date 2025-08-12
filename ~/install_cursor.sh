#!/bin/bash

echo "==================================="
echo "Cursor IDE 간단 설치 스크립트"
echo "==================================="

# Downloads 폴더에서 Cursor AppImage 찾기
CURSOR_FILE=$(ls ~/Downloads/cursor*.AppImage 2>/dev/null | head -1)

if [ -z "$CURSOR_FILE" ]; then
    echo "❌ ERROR: Cursor AppImage를 찾을 수 없습니다!"
    echo ""
    echo "📥 설치 방법:"
    echo "1. https://cursor.sh 웹사이트 방문"
    echo "2. 'Download for Linux' 버튼 클릭"
    echo "3. 다운로드 완료 후 이 스크립트 다시 실행"
    exit 1
fi

echo "✅ Cursor 파일 발견: $CURSOR_FILE"

# 실행 권한 부여
chmod +x "$CURSOR_FILE"

# 홈 디렉토리에 복사
cp "$CURSOR_FILE" ~/cursor.AppImage

echo ""
echo "==================================="
echo "✅ 설치 완료!"
echo "==================================="
echo ""
echo "🚀 Cursor 실행 방법:"
echo "   ~/cursor.AppImage"
echo ""
echo "💡 터미널에서 'cursor' 명령어로 실행하려면:"
echo "   sudo ln -s ~/cursor.AppImage /usr/local/bin/cursor"