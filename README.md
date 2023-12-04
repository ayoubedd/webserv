# Webserv

## Project Overview

This project is an exploration of the HTTP protocol by building a custom HTTP server in C++98. It provides a hands-on experience in understanding the intricacies of web communication, allowing users to test the server with a browser. The project covers both mandatory and bonus features, emphasizing non-blocking operations, configuration files, and CGI support.

### Table of Contents
- [Introduction](#introduction)
- [General Rules](#general-rules)
- [Mandatory Part](#mandatory-part)
- [Configuration File](#configuration-file)
- [Bonus Part](#bonus-part)
- [Submission and Peer-Evaluation](#submission-and-peer-evaluation)

## Introduction

The Hypertext Transfer Protocol (HTTP) is the foundation of data communication for the World Wide Web. This project delves into the development of an HTTP server in C++ 98, providing insights into web server functionalities, communication protocols, and server-client interactions.

## General Rules

- Ensure program stability and non-blocking behavior.
- Use a Makefile.
- Compile with c++ and -Wall -Wextra -Werror flags.
- Comply with C++ 98 standard.
- Avoid external libraries.

## Mandatory Part

- Program named 'webserv.'
- Accepts a configuration file as an argument.
- Non-blocking, single poll() for all I/O operations.
- Comprehensive error handling and accurate HTTP response codes.
- Static website serving, file uploads, and support for GET, POST, DELETE methods.
- Stress test to ensure server availability.

## Configuration File

- Choose port, host, and server_names.
- Set default error pages, limit client body size.
- Configure routes with various rules.
- Execute CGI based on file extension.
- Handle uploaded files and specify save locations.

## Bonus Part

- Support cookies and session management.
- Handle multiple CGI.

## Getting Started

## Building the project

To build the project, use the following command:

```sh
make
```

or faster:


```sh
make -j $(nproc)
```

## Startin the project
To run the server, use the following command:

```sh
./webserv [configuration file]
```

## License

This project is licensed under MIT license. See the LICENSE file for details.

## Appendix

The completion of the comprehensive project outlined in the introductory guidelines is detailed in the file located at subject/en.subject.pdf.
