## Overview

This example app demonstrates the following features:
- Retrieve orders from the database that match the specified search filter.

## How to use the sample

When you run the example app, all orders from the database that contain the `searchTerm` will be retrieved. The `searchTerm` is a string used to search for orders, and it is matched against each field in the order object.

## How it works

1. Initialize `vrp::Customer` objects with desired coordinates and details.
2. Create an `vrp::Order` object for each customer, setting the relevant order details.
3. Create a `ProgressListener`, `vrp::Service`, and an `vrp::OrderList`.
4. Add the initialized orders to the database by calling the `addOrder()` method from the `vrp::Service`, and wait for the operation to complete.
5. After adding the orders, call the `getOrders()` method from the `vrp::Service` using the list from step 3.) and the `ProgressListener`.
6. Once the operation completes, the list from step 3.) will be populated with orders that match the search criteria.
