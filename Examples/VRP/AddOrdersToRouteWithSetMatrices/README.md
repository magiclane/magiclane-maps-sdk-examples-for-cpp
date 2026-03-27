## Overview

This example app demonstrates the following features:
- Add a list of orders into an existing route's orders list, which has the matrices set by the user.

If the optimization is also reoptimized, a new solution will be returned which will include the added orders.
The orders will be also added in the list of orders of the route's optimization.

In this example the orders are added at the end of the route's list of orders. Check how to add the orders at some specified positions or at the optimal positions in [Add Orders To Route At Specified Positions](../AddOrdersToRouteAtSpecifiedPositions) and [Add Orders To Route At Optimal Positions](../AddOrdersToRouteAtOptimalPositions) examples.

## How to use the sample

When you run the example app, in addition to the existing orders, the route will also have the added orders.

## How it works

1. Create a `vrp::RouteOrder` with the desired fields for each order that will be added and insert them in a `vrp::RouteOrderList`.
2. Create one `IntListList` for the distances from the orders which will be added to the existing orders of the optimization and one `IntListList` for the durations.
3. Create a `ProgressListener` and `vrp::Service`.
4. Retrieve the route like in the example `GetRoute()` in a `vrp::Route`.
5. Call the `route.addOrders()` method from `vrp::Route` from 4.) using the list from 1.), a boolean to specify if the route should be reoptimized, a boolean to specify if the orders should be added at the optimal position (in this examples it has to be `false`), the lists from 2.) and the `ProgressListener`.
6. Once the operation completes, the `vrp::Route` from 4.) will be updated.
