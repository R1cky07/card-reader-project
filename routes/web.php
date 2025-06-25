<?php

use Illuminate\Support\Facades\Route;

Route::get('/admin/exist', function () {
    $token = cache()->get("token", false);

    return response()->json([
        "success" => true,
        "data" => [
            "token" => $token,
        ],
        "message" => ""
    ]);
});


Route::get('/admin/register', function () {
    $token = cache()->rememberForever("token", fn () => request()->input("token"));

    return response()->json([
        "success" => true,
        "data" => [
            "token" => $token,
        ],
        "message" => ""
    ]);
});


Route::get('/user/token', function () {
    $userToken = request()->input("token");
    $adminToken = cache()->get("token", false);
    $success = ($adminToken == $userToken);

    return response()->json([
        "success" => $success,
        "data" => [
            "token" => $userToken,  
        ],
        "message" => ""    
    ]);
});


Route::get('/user/register', function () {
    $userToken = cache()->rememberForever("userToken", fn () => request()->input("userToken"));
    $name = cache()->rememberForever("userName", fn () => request()->input("userName"));
    $surname = cache()->rememberForever("userSurname", fn () => request()->input("userSurname"));

    //Registrare nel database o una tabella da qualche parte
    //Controllare doppioni

    return response()->json([
       "success" => true,
       "data" => [
            "userToken" => $userToken,
            "userName" => $name,
            "userSurname" => $surname,
       ],
        "message" => ""
    ]);
});


Route::get('/user/checkin', function () {
    $token = request()->input("token");
    
    //Registrare da qualche parte
    return response()->json([
        "success" => true,
        "data" => [
            "token" => $token,
            "status" => "working",  
        ],
        "message" => ""
    ]);
});


Route::get('/user/checkout', function () {
    $token = request()->input("token");
    

    //Registrare da qualche parte
    return response()->json([
        "success" => true,
        "data" => [
            "token" => $token,
            "status" => "not working",  
        ],
        "message" => ""
    ]);
});

Route::get('/user/get', function () {
    $currentToken = request()->input("token");

    $adminToken = cache()->get("token", false);
    $userToken = cache()->get("userToken", false);
    
    return response()->json([
        "success" => true,
        "data" => [
            "userToken" => $userToken,
            "totalName" => cache()->get("userName", false) . " " . cache()->get("userSurname", false),
        ],
        "message" => ""
    ]);
});

Route::get('/user/test', function () {
    $currentToken = request()->input("token");

    $adminToken = cache()->get("token", false);
    $userToken = cache()->get("userToken", false);

    $isWorking = false;

    if ($currentToken == $adminToken || $currentToken == $userToken) {
        $isWorking = true;
    }

    return response()->json([
       "success" => $isWorking,
       "data" => [
            "token" => $currentToken,
       ],
       "message" => ""
    ]);
});