# C-Youtube-Downloader
building: `./build.bash`\
If you get error like: `Couldn't find <curl.curl.h> in include files`, you need to download curl dev package for your system:
##### Debian\Ubuntu\Raspberry Pi OS:
`sudo apt install libcurl4-openssl-dev`
##### Arch:
`sudo pacman -S curl`
##### Fedora\RHEL\CentOS\Rocky Linux:
`dnf install libcurl-devel`
##### openSUSE:
`sudo zypper install libcurl-devel`
### Usage:
`./ctube`\
Then it'll ask you to choose an option. Paste youtube link and it should work.\
