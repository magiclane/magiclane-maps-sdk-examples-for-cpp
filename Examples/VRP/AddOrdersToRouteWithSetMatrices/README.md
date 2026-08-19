## Overview

This example app demonstrates the following features:
- Add a list of orders into an existing route's orders list, which has the matrices set by the user.

If the optimization is also reoptimized, a new solution will be returned which will include the added orders.
The orders will be also added in the list of orders of the route's optimization.

In this example the orders are added at the end of the route's list of orders. Check how to add the orders at some specified positions or at the optimal positions in [Add Orders To Route At The Specified Positions](../AddOrdersToRouteAtTheSpecPos) and [Add Orders To Route At The Optimal Positions](../AddOrdersToRouteAtTheOptimPos) examples.

## How to use the sample

When you run the example app, in addition to the existing orders, the route will also have the added orders.

## How it works

1. Create a `vrp::RouteOrder` with the desired fields for each order that will be added and insert them in a `vrp::RouteOrderList`.
2. Create one `FloatListList` for the distances from the orders which will be added to the existing orders of the optimization and one `IntListList` for the durations.
3. Create a `ProgressListener`, `vrp::Service` and a `vrp::Request` that will be used to track the request status.
4. Retrieve the route like in the [Get Route](../GetRoute) example, in a `vrp::Route`.
5. Call the `route.addOrders()` method from `vrp::Route` from 4.) using, in this order: the `ProgressListener`, the list from 1.), a boolean to specify if the orders should be added at the optimal position (`false` here, so they go at the end), the `vrp::Request` from 3.), a boolean to specify if the route should be reoptimized (`false` here), and the lists from 2.). Each matrix is passed paired with the `vrp::EVehicleType` it applies to.
6. Once the operation completes, the orders have been added server-side. This sample then prints the `vrp::Route` object it fetched at 4.), which is the state from *before* the addition - call `getRoute()` again to see the updated route.
