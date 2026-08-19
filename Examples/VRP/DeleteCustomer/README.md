## Overview

This example app demonstrates the following features:
- Delete a customer from the agenda.

If the customer has orders in optimizations, they will be deleted.

## How to use the sample

When you run the example app, a customer will be deleted.

## How it works

1. Create a `ProgressListener` and `vrp::Service`.
2. Obtain the customer to delete. This sample creates a `vrp::Customer` and adds it with `addCustomer()` first; you can equally retrieve an existing one (see [Get Customer](../GetCustomer) example).
3. Call the `deleteCustomer()` method from the `vrp::Service` using a list of customer ids (`{ customer.getId() }`) and the `ProgressListener`, and wait for the operation to be done.
