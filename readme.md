```
┌──────────────────────────────────────┐
│   __  __     _ __  __     _ __    ♠A │
│  / / / /__  (_) / / /__  (_) /___ __ │
│ / /_/ / _ \/ / /_/ / _ \/ / __/ // / │
│ \____/_//_/_/\____/_//_/_/\__/\_, /  │
│                              /___/   │
│  A Student Group Optimizer           │
└──────────────────────────────────────┘
```

Given a list of student ids and id preferences, this application tries to find the most optimal grouping.

## Features

 - Chunked huffman compression/decompression
 - Stream cipher encryption/decryption
 - Genetic algorithm solver
 - Cool CLI
 - Bitshifting and ugly pointer management
 - No (known) memory leaks

As you can probably already guess, this was more of an "introduction project" to using C - Don't expect it to make toast or do backflips. Written 100% by yours truly as part of a 5 person group project. Yes, all 3678 lines.

## Running

```
make; ./main --help

usage: main [--help] [-d] ([-i] [-o])
optional arguments:
--help      show this help message and exit
-d          debug mode, shows additional data
-i          input csv of student preferences
-o          output csv of groups
```

eg:
```
./main -i import.csv -o results.csv
```