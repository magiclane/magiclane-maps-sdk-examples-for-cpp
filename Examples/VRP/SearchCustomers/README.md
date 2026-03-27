## Overview

This example app demonstrates the following features:
- Retrieve customers from the database that match the specified search filter.

## How to use the sample

When you run the example app, all the customers from database that contains `searchTerm` will be retrieved. The `searchTerm` is a string used to search for customers, and it is matched against each field in the customer's object.

## How it works

1. Create two `vrp::Customer` and set the desired fields.
2. Create a `ProgressListener` and `vrp::Service`and a `vrp::CustomerList`.
3. Call the `addCustomer()` method from the `vrp::Service` using the `vrp::Customer` and `ProgressListener` and wait for the operation to be done.
4. After adding the customers, call the `getAllCustomers()` method from the `vrp::Service` using the list from 2.) and the `ProgressListener`.
5. Once the operation completes, the list from 2.) will be populated with customers that match the search criteria.
