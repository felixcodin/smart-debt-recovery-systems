-- Enum types matching C++ model enums
CREATE TYPE employment_status_enum AS ENUM (
    'Employed',
    'Unemployed',
    'SelfEmployed',
    'Student',
    'Retired',
    'Contract',
    'PartTime',
    'None'
);

CREATE TYPE inactive_reason_enum AS ENUM (
    'PaidOff',
    'AccountClosed',
    'Deceased',
    'Fraud',
    'LegalAction',
    'None'
);

-- Borrowers table
CREATE TABLE IF NOT EXISTS borrowers (
    borrower_id SERIAL PRIMARY KEY,

    -- Personal Information
    first_name VARCHAR(50) NOT NULL,
    last_name VARCHAR(50) NOT NULL,

    -- Contact Information
    email VARCHAR(100) UNIQUE NOT NULL,
    phone_number VARCHAR(20),

    -- Address Information
    date_of_birth DATE,
    address TEXT,
    city VARCHAR(100),
    state VARCHAR(100),
    postal_code VARCHAR(20),
    country VARCHAR(100) DEFAULT 'Vietnam',

    -- Financial & Employment Information
    monthly_income DECIMAL(15, 2) DEFAULT 0,
    employment_status employment_status_enum DEFAULT 'None',

    -- Status & Lifecycle
    is_active BOOLEAN DEFAULT true NOT NULL,
    inactive_reason inactive_reason_enum DEFAULT 'None',
    inactivated_at TIMESTAMP,

    -- Audit Fields
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,

    -- Constraints
    CONSTRAINT valid_email CHECK (email ~* '^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}$'),
    CONSTRAINT valid_phone CHECK (phone_number IS NULL OR phone_number ~ '^[0-9]{10,11}$'),
    CONSTRAINT valid_income CHECK (monthly_income >= 0),
    CONSTRAINT inactive_logic CHECK (
        (is_active = true AND inactive_reason = 'None' AND inactivated_at IS NULL) OR
        (is_active = false AND inactive_reason != 'None' AND inactivated_at IS NOT NULL)
    )
);

-- Indexes for query performance
CREATE INDEX idx_borrowers_email ON borrowers(email);
CREATE INDEX idx_borrowers_is_active ON borrowers(is_active);
CREATE INDEX idx_borrowers_created_at ON borrowers(created_at DESC);
CREATE INDEX idx_borrowers_phone ON borrowers(phone_number) WHERE phone_number IS NOT NULL;

-- Function to auto-update updated_at timestamp
CREATE OR REPLACE FUNCTION update_updated_at_column()
RETURNS TRIGGER AS $$
BEGIN
    NEW.updated_at = CURRENT_TIMESTAMP;
    RETURN NEW;
END;
$$ LANGUAGE plpgsql;

-- Trigger for automatic updated_at
CREATE TRIGGER trigger_borrowers_updated_at
    BEFORE UPDATE ON borrowers
    FOR EACH ROW
    EXECUTE FUNCTION update_updated_at_column();

-- Table and column documentation
COMMENT ON TABLE borrowers IS 'Stores borrower/debtor information for the debt recovery system';
COMMENT ON COLUMN borrowers.borrower_id IS 'Primary key, auto-incrementing borrower ID';
COMMENT ON COLUMN borrowers.employment_status IS 'Current employment status (matches C++ EmploymentStatus enum)';
COMMENT ON COLUMN borrowers.inactive_reason IS 'Reason for inactive status (matches C++ InactiveReason enum)';
COMMENT ON COLUMN borrowers.inactivated_at IS 'Timestamp when borrower was marked as inactive';
COMMENT ON COLUMN borrowers.monthly_income IS 'Monthly income in USD (used for risk assessment and recovery strategies)';