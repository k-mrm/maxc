for file in `\find . -name '*.mxc'`; do
    echo -n $file :
    ../maxc $file
done
