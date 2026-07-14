# HTTP Configuration Note

The example API is configured to use **HTTP only** (not HTTPS) for simplicity in development.

## Configuration

**File:** `Logos.Payment.HttpHost/appsettings.json`

```json
{
  "Kestrel": {
    "Endpoints": {
      "Http": {
        "Url": "http://localhost:5000"
      }
    }
  }
}
```

**File:** `Logos.Payment.HttpHost/Program.cs`

HTTPS redirection is commented out:
```csharp
// Disable HTTPS redirection for development
// app.UseHttpsRedirection();
```

## Accessing the API

- **Swagger UI:** http://localhost:5000/swagger
- **API Base:** http://localhost:5000/api/

## For Production

In a production environment, you should:

1. **Enable HTTPS:**
   ```json
   "Kestrel": {
     "Endpoints": {
       "Https": {
         "Url": "https://localhost:5001"
       }
     }
   }
   ```

2. **Uncomment HTTPS redirection:**
   ```csharp
   app.UseHttpsRedirection();
   ```

3. **Use proper certificates** (not development certificates)

4. **Configure certificate sources** (Azure Key Vault, cert store, etc.)

## Why HTTP for Examples?

- **Simpler setup** - No certificate trust issues
- **Cross-platform** - Works consistently on all OSes
- **Focus on architecture** - SSL/TLS is an infrastructure concern (AP-007)
- **Easy testing** - curl, Postman, browsers work without warnings

The architectural patterns (layers, dependencies, use cases) are identical whether using HTTP or HTTPS. Transport security is a deployment/infrastructure concern, not an application architecture concern.
