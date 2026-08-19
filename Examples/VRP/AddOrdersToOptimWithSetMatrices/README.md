## Overview

This example app demonstrates the following features:
- Add a list of orders to an existing optimization which has custom matrices.

If the optimization is also reoptimized, a new solution will be returned which will include the added orders.

## How to use the sample

When you run the example app, in addition to the existing orders, the optimization will also have the added orders

## How it works

1. Create a `vrp::Order` with the desired fields for each order that will be added and insert them in a `vrp::OrderList`.
2. Create one `FloatListList` for the distances from the orders which will be added to the existing orders of the optimization and one `IntListList` for the durations.
3. Create a `ProgressListener` and `vrp::Service`.
4. Retrieve the optimization like in the [Get Optimization](../GetOptimization) example, in a `vrp::Optimization`.
5. Call the `optimization.addOrders()` method from `vrp::Optimization` from 4.), using the `ProgressListener`, the list from 1.), a `vrp::Request`, a boolean to specify if the optimization should be reoptimized (`true` here), and the lists from 2.). Each matrix is passed paired with the `vrp::EVehicleType` it applies to.
6. Once the operation completes, the `vrp::Optimization` from 4.) will be updated.
