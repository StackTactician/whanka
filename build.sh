#!/bin/bash
set -e

CC="${CC:-gcc}"
CFLAGS="-Wall -Wextra -std=c11 -O2"
LDFLAGS="-lm"
SRC="src/main.c src/lexer.c src/parser.c src/interpreter.c src/value.c src/env.c src/error.c"
OUT="pardon"

echo "Building Pardon language interpreter..."
echo "Compiler: $CC"

$CC $CFLAGS -o "$OUT" $SRC $LDFLAGS

strip "$OUT" 2>/dev/null || true

SIZE=$(ls -lh "$OUT" | awk '{print $5}')
echo "Built: ./$OUT ($SIZE)"

# Install to PATH
INSTALL_DIR=""
if [ -d "$HOME/.local/bin" ]; then
    INSTALL_DIR="$HOME/.local/bin"
elif [ -d "$PREFIX/bin" ]; then
    INSTALL_DIR="$PREFIX/bin"
elif [ -d "$HOME/bin" ]; then
    INSTALL_DIR="$HOME/bin"
else
    mkdir -p "$HOME/.local/bin"
    INSTALL_DIR="$HOME/.local/bin"
fi

cp "$OUT" "$INSTALL_DIR/$OUT"
chmod +x "$INSTALL_DIR/$OUT"
echo "Installed: $INSTALL_DIR/$OUT"

# Check if install dir is in PATH
case ":$PATH:" in
    *":$INSTALL_DIR:"*) ;;
    *)
        echo ""
        echo "NOTE: $INSTALL_DIR is not in your PATH."
        SHELL_RC=""
        if [ -f "$HOME/.bashrc" ]; then
            SHELL_RC="$HOME/.bashrc"
        elif [ -f "$HOME/.zshrc" ]; then
            SHELL_RC="$HOME/.zshrc"
        elif [ -f "$HOME/.profile" ]; then
            SHELL_RC="$HOME/.profile"
        fi

        if [ -n "$SHELL_RC" ]; then
            echo "export PATH=\"$INSTALL_DIR:\$PATH\"" >> "$SHELL_RC"
            echo "Added to $SHELL_RC. Run: source $SHELL_RC"
        else
            echo "Add this to your shell config:"
            echo "  export PATH=\"$INSTALL_DIR:\$PATH\""
        fi
        ;;
esac

echo ""
echo "Done. Run from anywhere: pardon <file.aids>"
