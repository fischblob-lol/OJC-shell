this is my own version of OJC-shell to include mac support (but mac only!)
# how 2 compile?
ok it uses meson to build
Heres how:
1. Clone the GitHub repository
```
git clone https://github.com/fischblob-lol/ojc-shell
```
2. Change Directory to the repository
```
cd ojc-shell
```
3. Run this (Presiquetes)
```
brew install pkg-config meson && brew link --force readline
```
> [!WARNING]
> brew link --force is a bit dangerous! You should watch out before using it.
4. Setup (run this in src)
```
meson setup <directory>
```
Now cd (change directory) into the directory that meson setup just made. i use meson setup ../build
5. Use 'ninja' to get the bin
```
ninja
```
6. Run it
```
./ojcsh
```
Wooho, you ran it on mac. how cool.
