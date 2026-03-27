## Overview

This example app demonstrates the following features:
- Delete a customer from the agenda.

If the customer has orders in optimizations, they will be deleted.

## How to use the sample

When you run the example app, a customer will be deleted.

## How it works

1. Create a `ProgressListener` and `vrp::Service`.
2. Call the `deleteCustomer()` method from the `vrp::Service` using the customer object and `ProgressListener` and wait for the operation to be done.
