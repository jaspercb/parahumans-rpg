# -*- mode: ruby -*-
# vi: set ft=ruby :
Vagrant.configure("2") do |config|
  config.vm.box = "ubuntu/xenial64"
  config.vm.provider "virtualbox" do |vb|
    vb.name = "parahumans-dev"
    vb.gui = true
    vb.memory = "2048"
  end

  config.vm.synced_folder "parahumans/", "/home/ubuntu/parahumans-dev"
  config.vm.boot_timeout = 600
  config.vm.provision "shell", inline: <<-SHELL
    apt-get -y update
    apt-get -y install lubuntu-desktop g++ zip git libsdl2-2.0 libsdl2-dev libgtest-dev cmake

    cd /usr/src/gtest
    sudo cmake CMakeLists.txt
    sudo make
    sudo cp *.a /usr/lib

    sudo add-apt-repository ppa:jonathonf/gcc-7.1
    sudo apt-get update
    sudo apt-get install gcc-7 g++-7

    passwd -d -u ubuntu

    reboot
  SHELL
end