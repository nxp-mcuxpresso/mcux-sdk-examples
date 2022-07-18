#!/bin/bash

# Copyright (c) Microsoft Corporation.
# Licensed under the MIT License.

# Bash script for creating ADU update import manifest.

ENABLE_VERBOSE=0

# This is the version of the ADU update manifest schema being used.
MANIFEST_VERSION='4.0'

print_usage() {
    printf "Usage: create-adu-import-manifest [OPTION]... [FILE]...
Create an import manifest to import an update into Device Update.

-p PROVIDER                 Update provider
-n NAME                     Update name
-v VERSION                  Update version
-c COMPATIBILITY_PROPERTY   Colon separated compatibility key value pair
                            May be specified multiple times.
-h HANDLER                  Instruction step handler
-r HANDLER_PROPERTY         Colon separated key value pair to be passed into handler
                            May be specified multiple times.
FILE                        Path to update file(s)
"
}

write_verbose() {
    if [[ $ENABLE_VERBOSE == 1 ]]; then
        local YELLOW
        local RESET

        YELLOW="$(tput setaf 11)"
        RESET="$(tput sgr0)"

        echo -e "${YELLOW}VERBOSE: $1${RESET}\n"
    fi
}

write_error() {
    local RED
    local RESET

    RED="$(tput setaf 9)"
    RESET="$(tput sgr0)"

    echo -e "${RED}ERROR: $1${RESET}\n"
}

#
# main
#

OPTIND=1

if [ $# -eq 0 ]; then
    print_usage
    exit 1
fi

if ! command -v openssl &> /dev/null
then
    write_error "This script requires openssl."
    exit 1
fi

# Parse commandline arguments
UPDATE_PROVIDER=''
UPDATE_NAME=''
UPDATE_VERSION=''
HANDLER=''
HANDLER_PROPS=()
COMPAT_INFOS=()
UPDATE_FILES=''

while getopts "c:r:n:p:h:v:" OPT; do
    case "$OPT" in
    c)
        if ! [[ "$OPTARG" =~ ^[^[:space:]]+:[^[:space:]]+$ ]]; then
            write_error "Invalid compatibility specified."
            exit 1
        fi

        IFS=':'
        read -ra ARGS <<<"$OPTARG"
        if [ ${#ARGS[@]} -ne 2 ]; then
            write_error 'Compatibility info format is key:value'
            exit 1
        fi

        # Multiple -c values are supported.
        COMPAT_INFOS+=("$OPTARG")
        ;;
    r)
        if ! [[ "$OPTARG" =~ ^[^[:space:]]+:[^[:space:]]+$ ]]; then
            write_error "Invalid handler property specified."
            exit 1
        fi

        IFS=':'
        read -ra ARGS <<<"$OPTARG"
        if [ ${#ARGS[@]} -ne 2 ]; then
            write_error 'Handler property format is key:value'
            exit 1
        fi

        # Multiple -r values are supported.
        HANDLER_PROPS+=("$OPTARG")
        ;;

    n)
        if [ ! -z "$UPDATE_NAME" ]; then
            write_error "Update name specified twice."
            exit 1
        fi

        UPDATE_NAME=$OPTARG

        if ! [[ "$UPDATE_NAME" =~ ^[a-zA-Z0-9.-]{1,64}$ ]]; then
            write_error "Invalid update name specified."
            exit 1
        fi
        ;;
    p)
        if [ ! -z "$UPDATE_PROVIDER" ]; then
            write_error "Update provider specified twice."
            exit 1
        fi

        UPDATE_PROVIDER=$OPTARG

        if ! [[ "$UPDATE_PROVIDER" =~ ^[a-zA-Z0-9.-]{1,64}$ ]]; then
            write_error "Invalid update provider specified."
            exit 1
        fi
        ;;
    h)
        if [ ! -z "$HANDLER" ]; then
            write_error "Handler specified twice."
            exit 1
        fi

        HANDLER=$OPTARG

        # POSIX regex only, sigh.
        if ! [[ "$HANDLER" =~ ^[^[:space:]]+/[^[:space:]]+:[[:digit:]]{1,5}$ ]]; then
            write_error "Invalid handler specified."
            exit 1
        fi
        ;;

    v)
        if [ ! -z "$UPDATE_VERSION" ]; then
            write_error "Update version specified twice."
            exit 1
        fi

        UPDATE_VERSION=$OPTARG
        ;;
    \?)
        print_usage
        exit 1
        ;;
    :)
        # Missing option argument
        print_usage
        exit 1
        ;;
    *)
        # Unimplemented option
        exit 1
        ;;
    esac
done

# Update files are the arguments without switches.
shift "$((OPTIND - 1))"
UPDATE_FILES=($@)

# Verify that all required arguments were specified and correct.

if [ -z "$UPDATE_NAME" ]; then
    write_error 'Update name not specified.'
    exit 1
fi

if [ -z "$HANDLER" ]; then
    write_error 'Handler not specified.'
    exit 1
fi

if [ -z "$UPDATE_PROVIDER" ]; then
    write_error 'Provider name not specified.'
    exit 1
fi

if [ -z "$UPDATE_VERSION" ]; then
    write_error 'Update version not specified.'
    exit 1
fi

if [ ${#COMPAT_INFOS[@]} -eq 0 ]; then
    write_error 'Compatibility info not specified.'
    exit 1
fi

if [ ${#UPDATE_FILES[@]} -eq 0 ]; then
    write_error 'Update file(s) not specified.'
    exit 1
fi

for file in "${UPDATE_FILES[@]}"; do
    if [ ! -f "$file" ]; then
        write_error "File '$file' not found."
        exit 1
    fi
done

CREATED_DATETIME=$(date --utc --iso-8601=seconds)

# Write out JSON.
# Using "jq" would be better, but trying to reduce script dependencies.

cat <<EOF
{
  "updateId": {
    "provider": "$UPDATE_PROVIDER",
    "name": "$UPDATE_NAME",
    "version": "$UPDATE_VERSION"
  },
  "isDeployable":  true,
  "compatibility": [
    {
EOF

for idx in "${!COMPAT_INFOS[@]}"; do
    IFS=':'
    read -ra ARGS <<<"${COMPAT_INFOS[$idx]}"

    if [ $((idx + 1)) -ne ${#COMPAT_INFOS[@]} ]; then
        cat <<EOF
      "${ARGS[0]}": "${ARGS[1]}",
EOF
    else
        cat <<EOF
      "${ARGS[0]}": "${ARGS[1]}"
EOF
    echo "    }"
    fi
done

cat <<EOF
  ],
  "instructions": {
    "steps": [
      {
        "type":  "inline",
        "handler": "$HANDLER",
        "files": [
EOF

for idx in "${!UPDATE_FILES[@]}"; do
    UPDATE_FILE="${UPDATE_FILES[$idx]}"
    FILENAME=$(basename "$UPDATE_FILE")

    if [ $((idx + 1)) -ne ${#UPDATE_FILES[@]} ]; then
        echo "          \"$FILENAME\","
    else
        echo "          \"$FILENAME\""
    fi
done

if [ ${#HANDLER_PROPS[@]} -ne 0 ]; then
    cat <<EOF
        ],
        "handlerProperties": {
EOF

    for idx in "${!HANDLER_PROPS[@]}"; do
        IFS=':'
        read -ra ARGS <<<"${HANDLER_PROPS[$idx]}"

        if [ $((idx + 1)) -ne ${#HANDLER_PROPS[@]} ]; then
            cat <<EOF
          "${ARGS[0]}": "${ARGS[1]}",
EOF
        else
            cat <<EOF
          "${ARGS[0]}": "${ARGS[1]}"
EOF
        echo "        }"
        fi
    done

else
    echo "        ]"

fi

cat <<EOF
      }
    ]
  },
  "files": [
EOF

for idx in "${!UPDATE_FILES[@]}"; do
    UPDATE_FILE="${UPDATE_FILES[$idx]}"
    FILENAME=$(basename "$UPDATE_FILE")
    FILESIZE=$(stat --printf="%s" "$UPDATE_FILE")
    SHA256HASH=$(openssl dgst -sha256 -binary "$UPDATE_FILE" | openssl base64)

    cat <<EOF
    {
      "filename": "$FILENAME",
      "sizeInBytes": $FILESIZE,
      "hashes": {
        "sha256": "$SHA256HASH"
      }
EOF
    if [ $((idx + 1)) -ne ${#UPDATE_FILES[@]} ]; then
        echo "    },"
    else
        echo "    }"
    fi
done

cat <<EOF
  ],
  "createdDateTime": "$CREATED_DATETIME",
  "manifestVersion": "$MANIFEST_VERSION"
}
EOF
