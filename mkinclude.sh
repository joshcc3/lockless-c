for i in `cd src && find . -type f -name "*.h" && cd ..`; do mkdir -p "include/$(dirname $i)"; cp src/$i include/$i; done
