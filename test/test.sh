fail=0

for file in `\find ./test -name '*.mxc'`; do
    echo -n $file :\ 
    ./maxc $file > /dev/null
    if [ $? -eq 0 ]; then
        echo "passed"
    else
        echo "failed"
        fail=1
    fi
done

if [ $fail -eq 0 ]; then
    echo "(*'-') < all passed"
    return 0;
else
    echo "(*-\"-) < test failed"
    return 1;
fi
