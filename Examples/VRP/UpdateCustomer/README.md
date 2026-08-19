## Overview

This example app demonstrates the following features:
- Make changes on a customer from the agenda.

## How to use the sample

When you run the example app, the customer changes will be saved.

## How it works

1. Create a `ProgressListener` and a `vrp::Service`.
2. Obtain the customer you want to update, in a `vrp::Customer`. This sample creates one and adds it with `addCustomer()`; you can equally retrieve an existing one (see [Get Customer](../GetCustomer) example).
3. Change the desired fields of the `vrp::Customer`.
4. Call the `updateCustomer()` method from the `vrp::Service` using the `ProgressListener` and the `vrp::Customer` from 2.), and wait for the operation to be done.
