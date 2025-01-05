# Installing Mending Wall

Install Mending Wall from your package manager by adding appropriate repository for your Linux distribution below. If your distribution is not listed, you can instead [install from source](#install-from-source).

??? success ":simple-ubuntu: Ubuntu 24.10 Oracular Oriole (amd64)"
    Install Doxide:
    ```
    echo 'deb http://download.indii.org/deb oracular main' | sudo tee /etc/apt/sources.list.d/indii.org.list
    curl -fsSL https://download.indii.org/deb/Release.key | gpg --dearmor | sudo tee /etc/apt/trusted.gpg.d/indii.org.gpg > /dev/null
    sudo apt update
    sudo apt install mendingwall
    ```

??? success ":simple-ubuntu: Ubuntu 24.04 Noble Numbat (amd64)"
    Install Doxide:
    ```
    echo 'deb http://download.indii.org/deb noble main' | sudo tee /etc/apt/sources.list.d/indii.org.list
    curl -fsSL https://download.indii.org/deb/Release.key | gpg --dearmor | sudo tee /etc/apt/trusted.gpg.d/indii.org.gpg > /dev/null
    sudo apt update
    sudo apt install mendingwall
    ```

??? success ":simple-debian: Debian 12 Bookworm (amd64)"
    Install Doxide:
    ```
    echo 'deb http://download.indii.org/deb bookworm main' | sudo tee /etc/apt/sources.list.d/indii.org.list
    curl -fsSL https://download.indii.org/deb/Release.key | gpg --dearmor | sudo tee /etc/apt/trusted.gpg.d/indii.org.gpg > /dev/null
    sudo apt update
    sudo apt install mendingwall
    ```

??? success ":simple-fedora: Fedora 40 (x86_64)"
    Install Doxide:
    ```
    sudo dnf config-manager --add-repo https://download.indii.org/rpm/fedora/40/indii.org.repo
    sudo dnf update
    sudo dnf install mendingwall
    ```

??? success ":simple-fedora: Fedora 39 (x86_64)"
    Install Doxide:
    ```
    sudo dnf config-manager --add-repo https://download.indii.org/rpm/fedora/39/indii.org.repo
    sudo dnf update
    sudo dnf install mendingwall
    ```

??? success ":simple-opensuse: openSUSE Tumbleweed (x86_64)"
    Install Doxide:
    ```
    sudo zypper addrepo https://download.indii.org/rpm/opensuse/tumbleweed/indii.org.repo
    sudo zypper refresh
    sudo zypper install mendingwall
    ```

## :fontawesome-solid-file-zipper: Install from source

If a Mending Wall package is not available for your operating system or you have special requirements, you can install from source. See the [README.md](https://github.com/lawmurray/mendingwall) file for instructions.
