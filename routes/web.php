<?php

use Illuminate\Support\Facades\Route;
use Illuminate\Support\Facades\Cache;

// Initializes the list of pre-defined unregistered users in cache if it doesn't exist.
function initializePredefinedUnregisteredUsers(): array
{
    return Cache::rememberForever('predefined_unregistered_users', function () {
        return [
            ['name' => 'Luca', 'surname' => 'Rossi'],
            ['name' => 'Marco', 'surname' => 'Bianchi'],
            ['name' => 'Giulia', 'surname' => 'Verdi'],
            ['name' => 'Anna', 'surname' => 'Neri'],
            ['name' => 'Francesco', 'surname' => 'Ferrari'],
            ['name' => 'Roberta', 'surname' => 'Gallo'],
            ['name' => 'Giulio', 'surname' => 'Esposito']
        ];
    });
}

// Retrieves all registered users from the cache.
function getUsersFromCache(): array
{
    return Cache::get('users', []);
}

// Saves the current state of registered users back to the cache.
function saveUsersToCache(array $users): void
{
    Cache::forever('users', $users);
}

// Checks if an admin token exists in the cache.
Route::get('/admin/exist', function () {
    $adminToken = Cache::get('admin_token', false);
    return response()->json([
        'success' => true,
        'data' => [
            'token' => $adminToken,
        ],
        'message' => ''
    ]);
});


// Registers an admin with the provided token, assigning 'Admin' as name and 'User' as surname by default if not explicitly provided.
Route::get('/admin/register', function () {
    $token = request()->input('token');
    $adminName = request()->input('adminName', 'Admin'); 
    $adminSurname = request()->input('adminSurname', 'User'); 

    if (!$token) {
        return response()->json(['success' => false, 'message' => 'Token parameter is required'], 400);
    }

    Cache::forever('admin_token', $token);

    $users = getUsersFromCache();
    $users[$token] = [
        'userToken' => $token,
        'userName' => $adminName,
        'userSurname' => $adminSurname,
        'type' => 'Admin',
        'isWorking' => false,
        'workedHours' => 0
    ];
    saveUsersToCache($users);

    return response()->json([
        'success' => true,
        'data' => [
            'token' => $token,
            'userName' => $adminName,
            'userSurname' => $adminSurname,
            'type' => 'Admin'
        ],
        'message' => 'Admin registered'
    ]);
});

// Retrieves information about a specific user by token, including their admin status.
Route::get('/user/token', function () {
    $userToken = request()->input('token');
    if (!$userToken) {
        return response()->json(['success' => false, 'message' => 'Token parameter is required'], 400);
    }

    $adminToken = Cache::get('admin_token', false);
    $isAdmin = ($userToken == $adminToken);

    $users = getUsersFromCache();
    $user = $users[$userToken] ?? null;

    if ($user) {
        $user['isAdmin'] = $isAdmin; 
        return response()->json([
            'success' => true,
            'data' => $user,
            'message' => ''
        ]);
    } else {
        return response()->json([
            'success' => false,
            'data' => null,
            'message' => 'User not found'
        ], 404);
    }
});

// Registers a new user with a token, name, and surname.
Route::get('/user/register', function () {
    $userToken = request()->input('userToken');
    $userName = request()->input('userName');
    $userSurname = request()->input('userSurname');

    if (!$userToken || !$userName || !$userSurname) {
        return response()->json(['success' => false, 'message' => 'All parameters (userToken, userName, userSurname) are required'], 400);
    }

    $users = getUsersFromCache();

    if (isset($users[$userToken])) {
        return response()->json([
            'success' => false,
            'message' => 'This card is already registered to a user.'
        ], 409);
    }

    $predefinedUnregistered = initializePredefinedUnregisteredUsers();
    $foundMatch = false;
    foreach ($predefinedUnregistered as $index => $unregUser) {
        if ($unregUser['name'] === $userName && $unregUser['surname'] === $userSurname) {
            $foundMatch = true;
            unset($predefinedUnregistered[$index]);
            Cache::forever('predefined_unregistered_users', array_values($predefinedUnregistered));
            break;
        }
    }

    if (!$foundMatch) {
        return response()->json([
            'success' => false,
            'message' => 'User name/surname not found in unregistered list or already registered.'
        ], 400);
    }


    $users[$userToken] = [
        'userToken' => $userToken,
        'userName' => $userName,
        'userSurname' => $userSurname,
        'type' => 'User', 
        'isWorking' => false, 
        'workedHours' => 0   
    ];
    saveUsersToCache($users);

    return response()->json([
        'success' => true,
        'data' => $users[$userToken],
        'message' => 'User registered successfully'
    ]);
});

// Checks a user into work by updating their 'isWorking' status to true.
Route::get('/user/checkin', function () {
    $token = request()->input('token');
    if (!$token) {
        return response()->json(['success' => false, 'message' => 'Token parameter is required'], 400);
    }

    $users = getUsersFromCache();

    if (!isset($users[$token])) {
        return response()->json(['success' => false, 'message' => 'User not found'], 404);
    }

    if ($users[$token]['isWorking']) {
        return response()->json(['success' => false, 'message' => 'User already checked in'], 400);
    }

    $users[$token]['isWorking'] = true;
    saveUsersToCache($users);

    return response()->json(['success' => true, 'message' => 'Checked in successfully']);
});

// Checks a user out of work by updating their 'isWorking' status to false and adding worked hours.
Route::get('/user/checkout', function () {
    $token = request()->input('token');
    if (!$token) {
        return response()->json(['success' => false, 'message' => 'Token parameter is required'], 400);
    }

    $users = getUsersFromCache();

    if (!isset($users[$token])) {
        return response()->json(['success' => false, 'message' => 'User not found'], 404);
    }

    if (!$users[$token]['isWorking']) {
        return response()->json(['success' => false, 'message' => 'User already checked out'], 400);
    }

    $users[$token]['isWorking'] = false;
    $users[$token]['workedHours'] += 4; // Assuming a fixed 4 hours for simplicity as in original code
    saveUsersToCache($users);

    return response()->json(['success' => true, 'message' => 'Checked out successfully']);
});

// Retrieves full details for a single user by token.
Route::get('/user/get', function () {
    $userToken = request()->input('token');
    if (!$userToken) {
        return response()->json(['success' => false, 'message' => 'Token parameter is required'], 400);
    }

    $users = getUsersFromCache();

    if (isset($users[$userToken])) {
        return response()->json([
            'success' => true,
            'data' => $users[$userToken],
            'message' => ''
        ]);
    } else {
        return response()->json([
            'success' => false,
            'data' => null,
            'message' => 'User not found'
        ], 404);
    }
});

// Retrieves a list of all registered users (excluding the admin if type 'Admin').
Route::get('/user/list', function () {
    $users = getUsersFromCache();
    $registeredUsers = [];
    foreach ($users as $uid => $userData) {
        if (isset($userData['type']) && $userData['type'] === 'User') { // Exclude admin from this list if you want a list of only 'regular' users
            $registeredUsers[] = $userData;
        }
    }
    return response()->json([
        'success' => true,
        'data' => array_values($registeredUsers), 
        'message' => ''
    ]);
});

// Retrieves a list of users who are not yet registered with a token.
Route::get('/user/unregistered-list', function () {
    $predefinedUnregistered = initializePredefinedUnregisteredUsers();
    $registeredUsers = getUsersFromCache();

    $filteredUnregistered = array_filter($predefinedUnregistered, function($unregUser) use ($registeredUsers) {
        foreach ($registeredUsers as $regUser) {
            if ($regUser['userName'] === $unregUser['name'] && $regUser['userSurname'] === $unregUser['surname']) {
                return false; // This name/surname pair is already registered, so filter it out.
            }
        }
        return true; // This name/surname pair is not yet registered.
    });

    return response()->json([
        'success' => true,
        'data' => array_values($filteredUnregistered), 
        'message' => ''
    ]);
});

// Replaces an existing user's device token with a new one.
Route::get('/user/replace-device', function () {
    $oldToken = request()->input('oldToken');
    $newToken = request()->input('newToken');

    if (!$oldToken || !$newToken) {
        return response()->json(['success' => false, 'message' => 'Both oldToken and newToken parameters are required'], 400);
    }

    $users = getUsersFromCache();

    if (!isset($users[$oldToken])) {
        return response()->json(['success' => false, 'message' => 'Original user token not found'], 404);
    }

    if (isset($users[$newToken])) {
        return response()->json(['success' => false, 'message' => 'New token already in use by another user'], 409);
    }

    $userData = $users[$oldToken];
    $userData['userToken'] = $newToken; 
    unset($users[$oldToken]); 
    $users[$newToken] = $userData;

    saveUsersToCache($users);

    return response()->json([
        'success' => true,
        'data' => $userData,
        'message' => 'Device token replaced successfully'
    ]);
});
