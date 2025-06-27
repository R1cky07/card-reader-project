<?php

use Illuminate\Support\Facades\Route;

Route::get('/admin/exist', function () {                                        //Check if there is an admin
    $token = cache()->get('token', false);

    return response()->json([
        'success' => true,
        'data' => [
            'token' => $token,
        ],
        'message' => ''
    ]);
});


Route::get('/admin/register', function () {                                                 //Let an admin register
    $token = cache()->rememberForever('token', fn () => request()->input('token'));

    return response()->json([
        'success' => true,
        'data' => [
            'token' => $token,
        ],
        'message' => ''
    ]);
});


Route::get('/user/token', function () {                             //Check if a user is the admin
    $userToken = request()->input('token');
    $adminToken = cache()->get('token', false);
    $isAdmin = ($userToken == $adminToken);

    return response()->json([
        'success' => true,
        'data' => [
            'isAdmin' => $isAdmin,
        ],
        'message' => ''    
    ]);
});


Route::get('/user/register', function () {                                              //Register a User
    
    
    $registeredUser = [
        'token' => cache()->rememberForever('userToken', fn () => request()->input('userToken')),
        'name' =>  cache()->rememberForever('userName', fn () => request()->input('userName')),
        'surname' => cache()->rememberForever('userSurname', fn () => request()->input('userSurname'))
    ];

    return response()->json([
       'success' => true,
       'data' => [
            'userToken' => $registeredUser["token"],
            'userName' => $registeredUser["name"],
            'userSurname' => $registeredUser["surname"],
       ],
        'message' => ''
    ]);
});

Route::get('/user/list', function () {                                      //Give a list of non-registered Users

    return response()->json([
       'success' => true,
       'data' => userlist(),
        'message' => ''
    ]);
});


Route::get('/user/checkin', function () {                           //Register an entry
    $token = request()->input('token');
   
    return response()->json([
        'success' => true,
        'data' => [
            'token' => $token,
            'status' => 'working',  
        ],
        'message' => ''
    ]);
});


Route::get('/user/checkout', function () {                                              //Register an exit
    $token = request()->input('token');
    
    return response()->json([
        'success' => true,
        'data' => [
            'token' => $token,
            'status' => 'not working',  
        ],
        'message' => ''
    ]);
});


Route::get('/user/get', function () {                               //Get an user from the given token
    $adminToken = cache()->get('token', false);
    $userToken = cache()->get('userToken', false);
    
    return response()->json([
        'success' => true,
        'data' => [
            'userToken' => $userToken,
            'totalName' => cache()->get('userName', false) . ' ' . cache()->get('userSurname', false),
        ],
        'message' => ''
    ]);
});


function userlist() {                                           //Show the list of users
    return [
        'User1' => [
            'userName' => 'Luca',
            'userSurname' => 'Rossi',
            'userToken' => ''
        ],
        'User2' => [
            'userName' => 'Piero',
            'userSurname' => 'Verdi',
            'userToken' => ''   
        ],
        'User3' => [
            'userName' => 'Chiara',
            'userSurname' => 'Bianchi',
            'userToken' => ''  
        ],
    ]; 
};