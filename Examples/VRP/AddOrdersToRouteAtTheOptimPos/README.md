## Overview

This example app demonstrates the following features:
- Add a list of orders into an existing route's orders list, at the optimal positions, which are determined by the algorithm.

The orders are inserted at the best positions between the route's orders, without rearranging the route's orders.
The route can be reoptimized, which means that after the addition, the route orders will be rearranged in the best order of visit, so you won't see the orders being added at the optimal positions.
The orders will be also added in the list of orders of the route's optimization.

## How to use the sample

When you run the example app, in addition to the existing orders, the route will also have the added orders.

## How it works

1. Create a `vrp::RouteOrder` with the desired fields for each order that will be added and insert them in a `vrp::RouteOrderList`.
2. Create a `ProgressListener` and `vrp::Service`.
3. Retrieve the route like in the example `GetRoute()` in a `vrp::Route`.
4. Call the `route.addOrders()` method from `vrp::Route` using the list from 1.), a boolean to specify if the route should be reoptimized, a boolean to specify if the order should be added at the optimal position (in this examples it has to be `true`) and the `ProgressListener`.
5. Check if the associated request has reached the finished status. Once completed, you can retrieve the updated route by calling the `getRoute()` method, which returns a `vrp::Route` containing the updated route.