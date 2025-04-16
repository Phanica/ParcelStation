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

## Response Format

```
{
    "status" = "<string>",
    "message" = "<string>",
}
```
