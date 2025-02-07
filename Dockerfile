from gcc:12.4 as builder

workdir /build

copy . .

run make clean && make all -j4

run cp .artifacts/dist/lambda.so /usr/local/lib/
run cp .artifacts/dist/lambda.a /usr/local/lib/

run cp ./lambda.hpp ./index.hpp

run find . -type d | grep -vw '.git' | sed 's/\.\///' | xargs -I {} mkdir -p /usr/local/include/maddsua/lambda/{}
run find . -name "*.hpp" | sed 's/\.\///' | xargs -I {} cp {} /usr/local/include/maddsua/lambda/{}
run find /usr/local/include/maddsua/lambda/ -type d -empty -delete

from scratch

copy --from=builder /usr/local/lib/lambda.so /usr/local/lib/
copy --from=builder /usr/local/lib/lambda.a /usr/local/lib/
copy --from=builder /usr/local/include/maddsua /usr/local/include/maddsua
