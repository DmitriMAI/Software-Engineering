cd task2/ // если из основной директории.
docker-compose up -d  
docker-compose down   
  
Как перегенеить сервис с зависимостями? (Сейчас без postresql, mongo, grpc)  
Зайти в контейнер запущенный докера, запустить команду, дождаться создания, выйти на уровень выше по папкам `cd ..` и выполнить без [] ныжные.
userver-create-service [--grpc] [--mongo] [--postgresql] myservice

Это запускать в контейнере generating
`pip install ruff`
`userver-create-service --postgresql myservice`



// Теперь попытки как заставить vs code определять зависимости.
// Ниже шаги что делал, но видимо надо - deb пакет зависимостей и clangd установить

// Это как будно не обзятельно было
sudo apt install \
    libboost-all-dev \
    libfmt-dev \
    libssl-dev \
    libc-ares-dev \
    libcctz-dev \
    libyaml-cpp-dev \
    libcrypto++-dev \
    libev-dev \
    libcurl4-openssl-dev \
    zlib1g-dev \
    libprotoc-dev \
    protobuf-compiler

// Попытка скачать зависимости, но не оч удачно
 lsb_release -a
 https://userver.tech/de/db9/md_en_2userver_2build_2dependencies.html

// Еще одна попытка
DEPS_FILE="https://raw.githubusercontent.com/userver-framework/userver/refs/heads/develop/scripts/docs/en/deps/ubuntu-22.04.md" && \
sudo apt install --allow-downgrades -y $(wget -q -O - ${DEPS_FILE})


// Вот тут уже было ок, но надо было скачать зависимости из этого deb и дальше запустить его install
https://github.com/userver-framework/userver/releases
Скачать 
wget https://github.com/userver-framework/userver/releases/download/v2.15/ubuntu22.04-libuserver-all-dev_2.15_amd64.deb

sudo apt install ./ubuntu22.04-libuserver-all-dev_2.15_amd64.deb 
sudo cp ubuntu22.04-libuserver-all-dev_2.15_amd64.deb /tmp/
sudo apt install /tmp/ubuntu22.04-libuserver-all-dev_2.15_amd64.deb
// Тут заработал частично headers 

pkg-config --libs --cflags userver

// image в этой директории попытка заменить на c++ 17 версии для c++ расширения
g++ --version
// Тут должен быть 17....
Скрин из директории - внутри расширения с++ 

// Тут Кирилл подсказал, что clang норм работает
sudo apt install clang

https://clangd.llvm.org/installation.html

sudo apt-get install clangd-12

sudo update-alternatives --install /usr/bin/clangd clangd /usr/bin/clangd-12 100
// после установки в целом начал находить все и подсвечивать правильно.

// Заставляем видеть все clangd сгенеренное и скачанное
// Нужно при этом зайти в директорию билда (у меня на винде cmake-build-debug-wsl)
mkdir cmake-build-debug-wsl
cd cmake-build-debug-wsl
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..
cd ..
ln -s ./cmake-build-debug-wsl/compile_commands.json compile_commands.json

pip install conan

conda install conda-forge::jwt-cpp