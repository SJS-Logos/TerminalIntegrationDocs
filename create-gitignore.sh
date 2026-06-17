OUTPUT=".gitignore"

curl -s "https://raw.githubusercontent.com/github/gitignore/main/VisualStudio.gitignore" >"$OUTPUT"

cat >> "$OUTPUT" <<'EOF'

# Ignore all top-level directories

