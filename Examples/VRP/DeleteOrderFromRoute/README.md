## Overview

This example app demonstrates the following features:
- Delete a order from a route.

It will also be deleted from the orders list of the route's optimization. It cannot be deleted if it is the start order of the route or the route's optimization. If you still want to delete it, first change the route's start order (using the method `UpdateRoute()`) and after you can delete it.

## How to use the sample

When you run the example app, the order will be deleted and the route will be updated.

## How it works

1. Create a `ProgressListener` and a `vrp::Service`.
2. Retrieve the route like in the [Get Route](../GetRoute) example, in a `vrp::Route`, using your own route id (the sample has a `-1` placeholder that must be replaced).
3. Take the `vrp::RouteOrder` you want to delete from `route.getOrders()`. This sample uses the order at index 4, and only runs if the route has more than four orders.
4. Call the `deleteOrder()` method from `vrp::Route` from 2.) using the `vrp::RouteOrder` from 3.) and the `ProgressListener`.
5. Once the operation completes, the `vrp::Route` from 2.) will be updated, and the sample prints it with its remaining orders on the console.
