set -euo pipefail

ROOT="${1:-.}"

for dir in src include; do
  if [ -d "$ROOT/$dir" ]; then
    find "$ROOT/$dir" -type f \
      ! -name 'glad.c' \
      ! -name 'glad.h' \
      ! -name 'khrplatform.c' \
      ! -name 'khrplatform.h' \
      -print0 | while IFS= read -r -d '' file; do
        rel="${file#"$ROOT"/}"
        echo "==================== $rel ===================="
        cat "$file"
        echo
    done
  fi
done
