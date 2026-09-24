a=`cat package.json | grep "chart.js" | cut -d ':' -f 2 | cut -d '"' -f 2`
if
    [[ "$a" = "2.9.4" ]] ; then echo "True"
    else 
    echo "False"
    echo $a
fi