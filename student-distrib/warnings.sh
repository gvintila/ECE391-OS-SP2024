# Run dos2unix warnings.sh then run ./warnings.sh and you will get all your warnings outputted in log.txt

#!/bin/bash
make clean && make dep
sudo make &> log.txt