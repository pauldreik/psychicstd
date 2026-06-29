#!/bin/bash
set -e

# Export current branch commits as patches + helper script in a tarball

BRANCH=$(git rev-parse --abbrev-ref HEAD)
TMPDIR=$(mktemp -d)
PATCH_DIR="$TMPDIR/patches"
mkdir -p "$PATCH_DIR"

# Count commits ahead of origin/main
COMMIT_COUNT=$(git rev-list --count origin/main.."$BRANCH")

if [ "$COMMIT_COUNT" -eq 0 ]; then
  echo "Error: No commits ahead of origin/main on branch '$BRANCH'"
  rm -rf "$TMPDIR"
  exit 1
fi

echo "Exporting $COMMIT_COUNT commits from '$BRANCH' (ahead of origin/main)..."

# Generate patches
git format-patch origin/main -o "$PATCH_DIR" >/dev/null

# Create helper script for receiver
cat >"$PATCH_DIR/apply-patches.sh" <<'HELPER_SCRIPT'
#!/bin/bash
set -e

# Apply patches from sneakernet distribution

if ! git rev-parse --git-dir > /dev/null 2>&1; then
    echo "Error: Not in a git repository"
    exit 1
fi

PATCH_COUNT=$(ls -1 *.patch 2>/dev/null | wc -l)

if [ "$PATCH_COUNT" -eq 0 ]; then
    echo "Error: No .patch files found in current directory"
    exit 1
fi

echo "Found $PATCH_COUNT patch(es) to apply"
echo ""

FAILED=0
for patch in $(ls -1 *.patch | sort); do
    echo "Applying $patch..."
    if ! git am "$patch"; then
        echo "ERROR: Failed to apply $patch"
        echo ""
        echo "To resolve:"
        echo "  1. Fix the conflicts in your working tree"
        echo "  2. Run: git add ."
        echo "  3. Run: git am --continue"
        echo ""
        echo "Or to skip this patch:"
        echo "  git am --skip"
        FAILED=1
        break
    fi
done

if [ "$FAILED" -eq 0 ]; then
    echo ""
    echo "✓ All patches applied successfully!"
else
    echo ""
    echo "⚠ Patch application stopped. Resolve conflicts and continue with 'git am --continue'"
    exit 1
fi
HELPER_SCRIPT

chmod +x "$PATCH_DIR/apply-patches.sh"

# Create README for the receiver
cat >"$PATCH_DIR/README.txt" <<'README_TEXT'
SNEAKERNET PATCH DISTRIBUTION
==============================

Contents:
- *.patch files (one per commit)
- apply-patches.sh (helper script)

To apply these patches:

1. Navigate to your git repository
2. Copy all files from this directory (patches and apply-patches.sh)
3. Run: ./apply-patches.sh

The patches will be applied in order. If a conflict occurs, resolve it
and run: git am --continue

For more details on applying patches manually:
  git am *.patch

README_TEXT

# Create tarball with descriptive name
TIMESTAMP=$(date +%Y%m%d-%H%M%S)
TARBALL="patches-${BRANCH}-${TIMESTAMP}.tar.gz"

tar -czf "$TARBALL" -C "$TMPDIR" patches/

# Cleanup
rm -rf "$TMPDIR"

echo "✓ Created: $TARBALL"
echo ""
echo "Contents:"
tar -tzf "$TARBALL" | head -20
TOTAL=$(tar -tzf "$TARBALL" | wc -l)
if [ "$TOTAL" -gt 20 ]; then
  echo "... and $((TOTAL - 20)) more files"
fi

echo ""
echo "═══════════════════════════════════════════════════════════════"
echo "NEXT STEPS:"
echo "═══════════════════════════════════════════════════════════════"
echo ""
echo "1. Transfer the tarball to your destination:"
echo "   • Copy to USB/external drive: cp $TARBALL /path/to/usb/"
echo "   • Share via email/cloud: upload $TARBALL"
echo ""
echo "2. On the receiving end:"
echo "   tar -xzf $TARBALL"
echo "   cd patches"
echo "   ./apply-patches.sh"
echo ""
echo "The helper script will apply all $COMMIT_COUNT commits in order."
echo "═══════════════════════════════════════════════════════════════"
