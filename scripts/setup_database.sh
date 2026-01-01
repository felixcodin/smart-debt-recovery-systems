#!/bin/bash
# setup_database.sh - Initialize PostgreSQL database for SDRS

set -e

# Default values (can be overridden by environment variables)
DB_HOST="${SDRS_DB_HOST:-localhost}"
DB_PORT="${SDRS_DB_PORT:-5432}"
DB_NAME="${SDRS_DB_NAME:-sdrs}"
DB_USER="${SDRS_DB_USER:-sdrs_user}"
DB_PASSWORD="${SDRS_DB_PASSWORD:-sdrs_password}"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
SCHEMA_FILE="$PROJECT_ROOT/database/schema.sql"

echo "============================================"
echo "  Smart Debt Recovery System - DB Setup"
echo "============================================"
echo ""
echo "Configuration:"
echo "  Host:     $DB_HOST"
echo "  Port:     $DB_PORT"
echo "  Database: $DB_NAME"
echo "  User:     $DB_USER"
echo ""

# Check if psql is available
if ! command -v psql &> /dev/null; then
    echo "ERROR: psql command not found."
    echo "Please install PostgreSQL client tools."
    exit 1
fi

# Check if schema file exists
if [ ! -f "$SCHEMA_FILE" ]; then
    echo "ERROR: Schema file not found: $SCHEMA_FILE"
    exit 1
fi

echo "Step 1: Creating database user and database..."
echo ""

# Create user and database using postgres superuser
# Note: This requires sudo access or postgres user permissions
cat << EOF | sudo -u postgres psql 2>/dev/null || psql -U postgres -h $DB_HOST -p $DB_PORT 2>/dev/null || {
    echo "Cannot connect as postgres user. Please run these commands manually:"
    echo ""
    echo "  CREATE USER $DB_USER WITH PASSWORD '$DB_PASSWORD';"
    echo "  CREATE DATABASE $DB_NAME OWNER $DB_USER;"
    echo "  GRANT ALL PRIVILEGES ON DATABASE $DB_NAME TO $DB_USER;"
    echo ""
}
-- Create user if not exists
DO \$\$
BEGIN
    IF NOT EXISTS (SELECT FROM pg_roles WHERE rolname = '$DB_USER') THEN
        CREATE USER $DB_USER WITH PASSWORD '$DB_PASSWORD';
    END IF;
END
\$\$;

-- Create database if not exists
SELECT 'CREATE DATABASE $DB_NAME OWNER $DB_USER'
WHERE NOT EXISTS (SELECT FROM pg_database WHERE datname = '$DB_NAME')\gexec

-- Grant privileges
GRANT ALL PRIVILEGES ON DATABASE $DB_NAME TO $DB_USER;
EOF

echo ""
echo "Step 2: Running schema migration..."
echo ""

# Run schema file
export PGPASSWORD="$DB_PASSWORD"
psql -h "$DB_HOST" -p "$DB_PORT" -U "$DB_USER" -d "$DB_NAME" -f "$SCHEMA_FILE"

echo ""
echo "Step 3: Verifying tables..."
echo ""

# Verify tables were created
psql -h "$DB_HOST" -p "$DB_PORT" -U "$DB_USER" -d "$DB_NAME" -c "\dt"

echo ""
echo "============================================"
echo "  Database setup completed successfully!"
echo "============================================"
echo ""
echo "To run services with database enabled:"
echo ""
echo "  export SDRS_USE_DATABASE=1"
echo "  export SDRS_DB_HOST=$DB_HOST"
echo "  export SDRS_DB_PORT=$DB_PORT"
echo "  export SDRS_DB_NAME=$DB_NAME"
echo "  export SDRS_DB_USER=$DB_USER"
echo "  export SDRS_DB_PASSWORD=$DB_PASSWORD"
echo ""
echo "Then start the services:"
echo "  ./scripts/start_all_services.sh"
echo ""
