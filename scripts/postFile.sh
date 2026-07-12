#!/usr/bin/env bash
# Upload a file to https://www.dreik.se/upload/.
#
# Usage:
#   scripts/postFile.sh FILE
#
# The upload endpoint and form field can be overridden if the site changes:
#   UPLOAD_URL=... UPLOAD_FIELD=... scripts/postFile.sh FILE
set -euo pipefail

UPLOAD_URL="${UPLOAD_URL:-https://www.dreik.se/upload/upload_file.php}"
UPLOAD_FIELD="${UPLOAD_FIELD:-file}"
UPLOAD_EMAIL="${UPLOAD_EMAIL:-}"
UPLOAD_DESCRIPTION="${UPLOAD_DESCRIPTION:-}"

if [ "$#" -ne 1 ]; then
  echo "Usage: $0 FILE" >&2
  exit 2
fi

FILE="$1"

if [ ! -f "$FILE" ]; then
  echo "Error: file does not exist: $FILE" >&2
  exit 1
fi

curl_args=(
  --fail
  --silent
  --show-error
  --location
  -F "MAX_FILE_SIZE=10485760000"
)

if [ -n "$UPLOAD_EMAIL" ]; then
  curl_args+=(-F "email=${UPLOAD_EMAIL}")
fi

if [ -n "$UPLOAD_DESCRIPTION" ]; then
  curl_args+=(-F "description=${UPLOAD_DESCRIPTION}")
fi

curl_args+=(-F "${UPLOAD_FIELD}=@${FILE}")
curl_args+=("$UPLOAD_URL")

curl "${curl_args[@]}"
