## Overview

This example app demonstrates the following features:
- Make changes on a customer from the agenda.

## How to use the sample

When you run the example app, the customer changes will be saved.

## How it works

1. Create a `ProgressListener` and a `vrp::Service`.
2. Retrieve the customer you want to update (see [Get Customer](../GetCustomer) example) in a `vrp::Customer`.
3. Change the desired fields of the `vrp::Customer`.
4. Call the `updateCustomer()` method from the `vrp::Service` using the `vrp::Customer` from 2.), a boolean to specify if the route will be reoptimized and the `ProgressListener` and wait for the operation to be done.
