Kevacoin Core integration/staging tree
=====================================

https://kevacoin.org

What is Kevacoin?
----------------

Kevacoin is a decentralized open source key-value data store based on Litecoin (which is in turn based on Bitcoin) cryptocurrency. Kevacoin is largely influenced by Namecoin [https://namecoin.org](https://namecoin.org), even though it serves very different purposes and works very differently.

What does it do?
----------------
* Securely record keys and their values. Size of value is up to 3072 bytes. No hard limits on the number of keys.
* Update or delete the keys and their values.
* Maintain network-unqiue namespaces. Keys are grouped under namespaces to avoid name conflicts.
* Transact the digital currency kevacoins (KVA).

What can it be used for?
------------------------
As a decentralized key-value database, it can be used to store data for all kinds of applications, such as social media, microblogging, public identity information, notary service. Kevacoin has limited support for smart contracts (similar to Bitcoin and Litecoin), but one can still develop decentralized apps (dApps) on Kevacoin. The data is decentralized while the application logic is developed off the blockchain.

Our major observation for decentralized apps is that data is significantly more important than the application. In fact, that is the case for all kinds of applications. It is common these days to hear that companies rewrite their applications using better technologies, but it is rare for any of them to make big changes to their valuable data.

For more information, as well as an immediately useable, binary version of
the Kevacoin Core software, see [https://kevacoin.org](https://kevacoin.org).

Documentation
--------------
* [Command line and RPC documentation](https://kevacoin.org/documentation.html)
* [Kevacoin ZeroMQ Event Notification](doc/zmq.md)


Build
-------------------

* [Unix](doc/build-unix.md)
* [Windows](doc/build-windows.md)
* [MacOS](doc/build-osx.md)

License
-------

Kevacoin Core is released under the terms of the MIT license. See [COPYING](COPYING) for more
information or see https://opensource.org/licenses/MIT.

Development Process
-------------------

The `master` branch is regularly built and tested, but is not guaranteed to be
completely stable. [Tags](https://github.com/kevacoin-project/kevacoin/tags) are created
regularly to indicate new official, stable release versions of Kevacoin Core.

The contribution workflow is described in [CONTRIBUTING.md](CONTRIBUTING.md).


Testing
-------

Testing and code review is the bottleneck for development; we need to carefully review and test the pull requests. Please be patient and help out by testing
other people's pull requests, and remember this is a security-critical project where any mistake might cost people money and data.

### Automated Testing

Developers are strongly encouraged to write [unit tests](src/test/README.md) for new code, and to
submit new unit tests for old code. Unit tests can be compiled and run
(assuming they weren't disabled in configure) with: `make check`. Further details on running
and extending unit tests can be found in [/src/test/README.md](/src/test/README.md).

There are also [regression and integration tests](/test), written
in Python, that are run automatically on the build server.
These tests can be run (if the [test dependencies](/test) are installed) with: `test/functional/test_runner.py`

The Travis CI system makes sure that every pull request is built for Windows, Linux, and OS X, and that unit/sanity tests are run automatically.

### Manual Quality Assurance (QA) Testing

Changes should be tested by somebody other than the developer who wrote the
code. This is especially important for large or high-risk changes. It is useful
to add a test plan to the pull request description if testing the changes is
not straightforward.

[gitter-image]: http://img.shields.io/badge/+%20GITTER-JOIN%20CHAT%20%E2%86%92-1DCE73.svg?style=flat-square
[gitter-url]: https://gitter.im/kevacoin-project/community
