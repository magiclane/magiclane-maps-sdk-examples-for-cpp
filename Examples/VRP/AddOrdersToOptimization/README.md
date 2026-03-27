## Overview

This example app demonstrates the following features:
- Add a list of orders to an existing optimization.

If the optimization is also reoptimized, the added orders will be assigned to the optimization's routes.

## How to use the sample

When you run the example app, in addition to the existing orders, the optimization will also have the added orders

## How it works

1. Create a `vrp::Order` with the desired fields for each order that will be added and insert them in a `vrp::OrderList`.
2. Create a `ProgressListener` and `vrp::Service`.
3. Retrieve the optimization like in the example `GetOptimization()` in a `vrp::Optimization`.
4. Call the `optimization.addOrders()` method from `vrp::Optimization` using the list from 1.), a boolean to specify if the optimization should be reoptimized and the `ProgressListener`.
5. Check if the associated request has reached the finished status. Once completed, you can retrieve the optimization results by calling the `getSolution()` method, which returns a `vrp::RouteList` containing the generated routes.
