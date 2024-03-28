# webserv

## Table of Contents
- [Overview](#overview)
- [Features](#features)
- [Requirements](#requirements)
- [Build](#build)
- [Usage](#usage)
    - [Example](#example)
- [Notes](#notes)
- [Lisence](#lisence)

## Overview

This project is an exploration of the HTTP protocol by building a custom HTTP server in C++98. It provides a hands-on experience in understanding the intricacies of web communication, allowing users to test the server with a browser. The project covers both mandatory and bonus features, emphasizing non-blocking operations, configuration files, and CGI support.

## Features

This is a list of features suported in this implementation.

- Redirections
- Template based error generation. 
- Event driven Non-Blocking single process/thread runtime.
- Method filtering
- Max headers size
- Max body size
- Directory listing
- Executing CGI scripts

## Requirements

To build this project you need the following:

- C++ compiler
- Make

## Build

To build this project run the following command:

```sh
make
```

This will result in a binary named `webserv`.

## Usage

To user `webserv` you need a configuration file describing one or more logical http servers.

And run as the following:

```sh
./webserv /path/to/config.conf
```

### Example

This is a example config to server a simple staic html website.

```txt
default www.example.com:5000 {
  error ./static/templates/errors.html;

  max_header_size 4096;

  route / {
    methods GET;

    root ./static/;
    index index.html; 
  }
}

```

This is a simple config file which will server static files from `./static` directory, and it only allows `GET` methods. with max headers size of 4096.

## Notes

- `config.conf` in the root directory contains the full configuration options.

## Lisence

This project is licensed under MIT license. See the LICENSE file for details.

