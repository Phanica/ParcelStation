## Postman Data Format

```
{
    "has_login" = "<boolean>",
    // if has_login is true, username and password are required
    "username" = "<string>",
    "password" = "<string>",
}
```

## Server Data Format

```
{
    "postman" = [
        // default user must be included
        {
            "username" = "default",
            "password" = "default",
        },
        // other users
        {
            "username" = "<string>",
            "password" = "<string>",
            "parcels" = [
                {
                    "id" = "<string>",
                    "parameter" = {
                        // ...
                    },
                },
            ],
        },
        // ...
    ],
    "manager" = [
        // just like postman
        {
            "username" = "default",
            "password" = "default",
        },
        {
            "username" = "<string>",
            "password" = "<string>",
            "parcels" = [],
        },
    ],
    "user" = [
        // just like postman
        {
            "username" = "default",
            "password" = "default",
        },
        {
            "username" = "<string>",
            "password" = "<string>",
            "parcels" = [],
        },
    ],
}
```

## Request Format

```
{
    "user" = {
        "type" = "<string>",
        "username" = "<string>",
        "password" = "<string>",
    },
    "request" = {
        "type" = "<string>",
        "arguments" = {
            // ...
        },
    },
}
```

## Request Format Alternative

All strings below are C-String, ending with '\0'
```
// metadata
    header_length: i32
    request_length: i32

// header (user_info)
    // metadata
        username_length: i32
        password_length: i32
    // data
        user_type_enum: i32
        username: string[username_length]
        password: string[password_length]

// request
    // metadata
        arguments_length: i32
    // data
        request_type_enum: i32
        arguments: bytes[arguments_length]
```

### Arguments Format

#### Signup / Login Arguments
```
// metadata
    username_length: i32
    password_length: i32

// data
    username: string[username_length]
    password: string[password_length]
```

#### Get / List Arguments

```
<delete>
```

#### Assign / Report_loss Arguments

```
// metadata
    task_id_length: i32

// data
    task_id: string[task_id_length]
```

## Response Format

```
{
    "status" = "<string>",
    "message" = "<string>",
}
```

## Response Format Alternative

```
// metadata
    message_length: i32

// data
    status_enum: i32
    message: bytes[message_length]
```

### Message Format

#### Signup / Login / Get / Assign / Report_loss Message

```
<delete>
```

#### List Message

```
// metadata
    index_length: i32

// data
    // index
        /** index contains a list, each element of which represents a length of parcel info */
        index: i32[index_length]
    // parcels
        /** parcels contains a list, each element of which represents a parcel info */
        parcels: bytes[index[i]]
```

##### Parcel Info Format

```
// metadata
    id_length: i32
    // other fields are waitting to be added

// data
    id: string[id_length]
    // other fields are waitting to be added
```
