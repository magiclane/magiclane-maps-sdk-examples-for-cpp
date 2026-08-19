## Overview

This example app demonstrates the following features:
- Get in which optimizations and routes has the customer made orders.

## How to use the sample

When you run the example app, the customer order history will be returned.

## How it works

1. Create a `ProgressListener`, a `vrp::Service` and a `vrp::CustomerOrderList`.
2. Retrieve the customer whose history you want with `getCustomer()`, into a `vrp::Customer`.
3. Call the `getCustomerOrdersHistory()` method from the `vrp::Service` using the `vrp::CustomerOrderList` from 1.), the `vrp::Customer` from 2.) and the `ProgressListener`.
4. Once the operation completes, the `vrp::CustomerOrderList` from 1.) will be populated.
