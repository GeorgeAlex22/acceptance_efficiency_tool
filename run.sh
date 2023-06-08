make
CONFIG=$1
VERBOSE=false
SAVE_FILE=true

# Parse command line arguments
shift  # Shift the positional arguments to skip the first argument ($1)

while [[ $# -gt 0 ]]; do
    case "$1" in
        -v|--verbose)
            VERBOSE=true
            shift
            ;;
        -s|--dont-save-file)
            SAVE_FILE=false
            shift
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Define string variables for the command line options
VERBOSE_S="false"
SAVE_FILES_S="true"

# Set the string variables based on the boolean values
if [ "$VERBOSE" = "true" ]; then
  VERBOSE_S="true"
fi

if [ "$SAVE_FILE" = "false" ]; then
  SAVE_FILES_S="false"
else
  mkdir -p rootfiles
fi

# Run the program with the specified options
# echo ./bin/main.exe "$CONFIG" $VERBOSE_S $SAVE_FILES_S
./bin/main.exe "$CONFIG" $VERBOSE_S $SAVE_FILES_S
