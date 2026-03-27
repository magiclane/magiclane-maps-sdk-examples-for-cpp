## Overview

This example app demonstrates the following features:
- Make changes on a route.

Besides the orders, all the other fields can be changed using the method `UpdateRoute()`; for changing the orders look over the examples: [Add Orders To Route](../AddOrdersToRoute), [Delete Order From Route](../DeleteOrderFromRoute) and [Update Order From Optimization](../UpdateOrderFromOptimization) examples.
The route can also be reoptimized, so a new order a visit will be returned based on the changes made.
The changes made over the route will not affect the route's optimization.

The change of the next configuration parameters automatically triggers the reoptimziation of the route:
- ignore time window
- optimization criterion
- optimization quality
- maximum time to wait at an order
- sequence pairs
- fixed orders sequences

## How to use the sample

When you run the example app, the new route will be returned.

## How it works

1. Create a `ProgressListener` and a `vrp::Service`.
2. Retrieve the route you want to update (see [Get Route](../GetRoute) example) in a `vrp::Route`.
3. Change the desired fields of the `vrp::Route`.
4. Call the `updateRoute()` method from the `vrp::Service` using the `vrp::Route` from 2.), a boolean to specify if the route will be reoptimized and the `ProgressListener`.
5. Check if the associated request has reached the finished status. Once completed, you can retrieve the updated route by calling the `getRoute()` method, which returns a `vrp::Route` containing the updated route.
