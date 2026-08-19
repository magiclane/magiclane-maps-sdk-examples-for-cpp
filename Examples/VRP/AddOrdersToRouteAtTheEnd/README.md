## Overview

This example app demonstrates the following features:
- Add a list of orders at the end of an existing route's orders list.

The route can be reoptimized, which means that after the addition, the route orders will be rearranged in the best order of visit, so you won't see the orders being added at the end of the route's orders list.
The orders will be also added in the list of orders of the route's optimization.

## How to use the sample

When you run the example app, in addition to the existing orders, the route will also have the added orders.

## How it works

1. Create a `vrp::RouteOrder` with the desired fields for each order that will be added and insert them in a `vrp::RouteOrderList`.
2. Create a `ProgressListener`, `vrp::Service` and a `vrp::Request` that will be used to track the request status.
3. Retrieve the route like in the [Get Route](../GetRoute) example, in a `vrp::Route`.
4. Call the `route.addOrders()` method from `vrp::Route` using, in this order: the `ProgressListener`, the list from 1.), a boolean to specify if the orders should be added at the optimal position (`false` here, so they go at the end), the `vrp::Request` from 2.), and a boolean to specify if the route should be reoptimized (`false` here).
5. Check if the associated request has reached the finished status. Once completed, the orders have been added server-side. This sample then prints the `vrp::Route` object it fetched at 3.), which is the state from *before* the addition - call `getRoute()` again to see the updated route, as [Update Route](../UpdateRoute) does.
