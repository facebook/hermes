# Experimental C based stable ABI for Hermes

This directory contains ongoing work to develop a stable C-based ABI for Hermes and an accompanying C++ JSI wrapper. It is a work in progress and is not supported for general use.

The goal of this ABI is to allow Hermes to be updated independently of the rest of a React Native application. Note that this does not immediately solve the general problem of ABI stability for RN extensions, since they are still written against the C++ JSI, and consume a C++ `jsi::Runtime` provided by RN. However, that is an eventual goal of this work.
