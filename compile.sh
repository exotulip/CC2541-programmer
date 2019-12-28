SRCFILES="main.c gpiolib.c"
gcc -o CC2541-programmer $SRCFILES
sudo setcap 'CAP_SYS_NICE=ep' "$(pwd)/CC2541-programmer"
