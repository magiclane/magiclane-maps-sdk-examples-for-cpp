## Overview

This example app demonstrates the following features:
- The orders are inserted at the specified positions between the route's orders.

The route can be reoptimized, which means that after the addition, the route orders will be rearranged in the best order of visit, so you won't see the orders being added at the specified positions.
The orders will be also added in the list of orders of the route's optimization.

## How to use the sample

When you run the example app, in addition to the existing orders, the route will also have the added orders.

## How it works

1. Create a `vrp::RouteOrder` for each order that will be added. Set the specified position at which it will be added using the method `setIndexInRoute()` and set other desired fields.
2. Insert all the `vrp::RouteOrder` created at 1.) in a `vrp::RouteOrderList`.
3. Create a `ProgressListener`, `vrp::Service` and a `vrp::Request` that will be used for traking the request status.
4. Retrieve the route like in the example `GetRoute()` in a `vrp::Route`.
5 Call the `route.addOrders()` method from `vrp::Route`, using the list from 2.), a boolean to specify if the route should be reoptimized, a boolean to specify if the orders should be added at the optimal position (in this examples it has to be `false`) and the `ProgressListener`.
6. Check if the associated request has reached the finished status. Once completed, you can retrieve the updated route by calling the `getRoute()` method, which returns a `vrp::Route` containing the updated route.
