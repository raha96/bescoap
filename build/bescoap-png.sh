SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
$SCRIPT_DIR/bescoap $1 $2.dot
dot -Tpng $2.dot -o $2.png