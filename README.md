# CS 460 Project

## Structure
The main library source files are in the `src` directory. The main function and tests for each stage are contained in their own directories, `comments`, `tokenizer`, `cst`, and `symbols`.

## Building
Depending on the stage, from the project root run:
```bash
cd {stage}
make

```

Once the stage has been built, it can be tested with
```bash
./test.sh
```

Or executed for a given input file with
```bash
./build/{stage} path/to/source/file.c
```