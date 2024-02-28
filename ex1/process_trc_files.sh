#!/bin/bash
let i=4; while((i<150)); do ./bp_main ./tests/example${i}.trc > ./tests/your_output_for_example${i}.out; let i++; done	