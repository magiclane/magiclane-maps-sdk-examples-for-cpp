## Overview

This example app demonstrates the following features:
- Get a certain customer from the agenda.

Check how to update customer data in [Update Customer](../UpdateCustomer) example, to display on the map in [Add Customer](../AddCustomer) example or to create an order and use it in an optimization, see the [Add Full Optimization](../AddFullOptimization) example.

## How to use the sample

When you run the example app, the customer will be returned.

## How it works

1. Create a `ProgressListener`, a `vrp::Service` and a `vrp::Customer`.
2. Call the `getCustomer()` method from the `vrp::Service` using the `vrp::Customer` from 1.), the ID of the customer that you want to retrieve and the `ProgressListener`.
3. Once the operation completes, the `vrp::Customer` from 1.) will be populated.
