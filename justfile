default:
    just --list

zip stage:
    cd {{stage}} && make clean && cd ..
    zip -r {{stage}}.zip {{stage}} src
    unzip -o {{stage}}.zip -d unzipped_{{stage}}
    cd unzipped_{{stage}}/{{stage}} && ./test.sh && cd .. && cd .. && rm -rf unzipped_{{stage}}
