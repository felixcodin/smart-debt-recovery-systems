-- ============================================================================
-- NOTE:
-- This schema includes CORE MVP, ADVANCED, and OPTIONAL features.
-- Application code (C++ microservices) will implement CORE MVP only.
-- ADVANCED/OPTIONAL tables are reserved for future extension or simulation.
-- ============================================================================

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
COMMENT ON TABLE borrowers IS '[CORE MVP] Stores borrower/debtor information for the debt recovery system';
COMMENT ON COLUMN borrowers.borrower_id IS 'Primary key, auto-incrementing borrower ID';
COMMENT ON COLUMN borrowers.employment_status IS 'Current employment status (matches C++ EmploymentStatus enum)';
COMMENT ON COLUMN borrowers.inactive_reason IS 'Reason for inactive status (matches C++ InactiveReason enum)';
COMMENT ON COLUMN borrowers.inactivated_at IS 'Timestamp when borrower was marked as inactive';
COMMENT ON COLUMN borrowers.monthly_income IS 'Monthly income in USD (used for risk assessment and recovery strategies)';

-- Account Status Enum
CREATE TYPE account_status_enum AS ENUM (
    'Current',
    'Delinquent',
    'Partial',
    'Default',
    'PaidOff',
    'ChargedOff',
    'Settled'
);

-- LoanAccounts table
CREATE TABLE IF NOT EXISTS loan_accounts (
    -- Primary Key
    account_id SERIAL PRIMARY KEY,

    -- Foreign Key
    borrower_id INTEGER NOT NULL REFERENCES borrowers(borrower_id) ON DELETE CASCADE,
    
    -- Loan Financial Details
    loan_amount DECIMAL(15, 2) NOT NULL,
    initial_amount DECIMAL(15, 2) NOT NULL,
    interest_rate DECIMAL(4, 3) NOT NULL,
    remaining_amount DECIMAL(15, 2) NOT NULL,

    -- Loan Terms
    loan_start_date TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    loan_end_date TIMESTAMP NOT NULL,

    -- Status & Delinquency
    account_status account_status_enum DEFAULT 'Current',
    days_past_due INTEGER DEFAULT 0,
    number_of_missed_payments INTEGER DEFAULT 0,

    -- Audit Fields
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,

    -- Constrains
    CONSTRAINT valid_amount CHECK (loan_amount > 0 AND remaining_amount >= 0),
    CONSTRAINT valid_interest CHECK (interest_rate >= 0 AND interest_rate <= 1),
    CONSTRAINT valid_missed_payments CHECK (number_of_missed_payments >= 0),
    CONSTRAINT valid_past_due CHECK (days_past_due >= 0),
    CONSTRAINT valid_dates CHECK (loan_end_date > loan_start_date)
);

CREATE INDEX idx_loan_accounts_borrower ON loan_accounts(borrower_id);
CREATE INDEX idx_loan_accounts_status ON loan_accounts(account_status);
CREATE INDEX idx_loan_accounts_past_due ON loan_accounts(days_past_due) WHERE days_past_due > 0;
CREATE INDEX idx_loan_accounts_created_at ON loan_accounts(created_at DESC);

CREATE TRIGGER trigger_loan_accounts_updated_at
    BEFORE UPDATE ON loan_accounts
    FOR EACH ROW
    EXECUTE FUNCTION update_updated_at_column();

COMMENT ON TABLE loan_accounts IS '[CORE MVP] Stores loan account information for the debt recovery system. Each row represents a loan given to a borrower.';
COMMENT ON COLUMN loan_accounts.account_id IS 'Primary key, auto-incrementing loan account ID';
COMMENT ON COLUMN loan_accounts.borrower_id IS 'Foreign key to borrowers table, identifies the borrower of this loan';
COMMENT ON COLUMN loan_accounts.loan_amount IS 'Original loan amount in USD';
COMMENT ON COLUMN loan_accounts.remaining_amount IS 'Current outstanding balance in USD';
COMMENT ON COLUMN loan_accounts.interest_rate IS 'Annual interest rate as decimal (0.05 = 5%)';
COMMENT ON COLUMN loan_accounts.account_status IS 'Current status of the loan account (matches C++ AccountStatus enum)';
COMMENT ON COLUMN loan_accounts.days_past_due IS 'Number of days payment is overdue, used for risk assessment and recovery strategies';

-- Payment Status Enum
CREATE TYPE payment_status_enum AS ENUM (
    'Pending',
    'Completed',
    'Failed',
    'Cancelled'
);

-- Payment Method Enum
CREATE TYPE payment_method_enum AS ENUM (
    'BankTransfer',
    'Cash',
    'Card',
    'Other'
);

-- Payment History Table
CREATE TABLE IF NOT EXISTS payment_history (
    payment_id SERIAL PRIMARY KEY,
    account_id INTEGER NOT NULL REFERENCES loan_accounts(account_id) ON DELETE CASCADE,
    
    payment_amount DECIMAL(15, 2) NOT NULL,
    payment_method payment_method_enum NOT NULL DEFAULT 'Cash',
    payment_status payment_status_enum NOT NULL DEFAULT 'Pending',
    
    payment_date DATE NOT NULL,
    due_date DATE,
    is_late BOOLEAN NOT NULL DEFAULT false,
    
    notes TEXT,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT valid_payment_amount CHECK (payment_amount > 0)
);

CREATE INDEX idx_payment_history_account ON payment_history(account_id);
CREATE INDEX idx_payment_history_status ON payment_history(payment_status);
CREATE INDEX idx_payment_history_date ON payment_history(payment_date DESC);

CREATE TRIGGER trigger_payment_history_updated_at
    BEFORE UPDATE ON payment_history
    FOR EACH ROW
    EXECUTE FUNCTION update_updated_at_column();

COMMENT ON TABLE payment_history IS '[CORE MVP] Stores payment history for loan accounts - tracks all payment transactions';

-- Risk Level Enum
CREATE TYPE risk_level_enum AS ENUM (
    'Low',
    'Medium',
    'High'
);

-- Risk Assessments Table
CREATE TABLE IF NOT EXISTS risk_assessments (
    assessment_id SERIAL PRIMARY KEY,
    account_id INTEGER NOT NULL REFERENCES loan_accounts(account_id) ON DELETE CASCADE,
    borrower_id INTEGER NOT NULL REFERENCES borrowers(borrower_id) ON DELETE CASCADE,
    
    risk_score DECIMAL(5, 3) NOT NULL,
    risk_level risk_level_enum NOT NULL,
    algorithm_used VARCHAR(50) NOT NULL DEFAULT 'RandomForest',
    risk_factors JSONB,
    
    assessment_date TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,

    CONSTRAINT valid_risk_score CHECK (risk_score >= 0 AND risk_score <= 1),
    CONSTRAINT risk_level_matches_score CHECK (
        (risk_level = 'Low' AND risk_score < 0.33) OR
        (risk_level = 'Medium' AND risk_score >= 0.33 AND risk_score < 0.67) OR
        (risk_level = 'High' AND risk_score >= 0.67)
    )
);

CREATE INDEX idx_risk_assessments_account ON risk_assessments(account_id);
CREATE INDEX idx_risk_assessments_risk_level ON risk_assessments(risk_level);
CREATE INDEX idx_risk_assessments_date ON risk_assessments(assessment_date DESC);

COMMENT ON TABLE risk_assessments IS '[CORE MVP - ML] Stores risk assessment results from RandomForest model, supports rule-based scoring fallback';

-- Strategy Type Enum
CREATE TYPE strategy_type_enum AS ENUM (
    'AutomatedReminder',
    'SettlementOffer',
    'LegalAction'
);

-- Strategy Status Enum
CREATE TYPE strategy_status_enum AS ENUM (
    'Pending',
    'Active',
    'Completed',
    'Failed',
    'Cancelled'
);

-- Recovery Strategies Table
CREATE TABLE IF NOT EXISTS recovery_strategies (
    strategy_id SERIAL PRIMARY KEY,
    account_id INTEGER NOT NULL REFERENCES loan_accounts(account_id) ON DELETE CASCADE,
    borrower_id INTEGER NOT NULL REFERENCES borrowers(borrower_id) ON DELETE CASCADE,
    
    strategy_type strategy_type_enum NOT NULL,
    strategy_status strategy_status_enum NOT NULL DEFAULT 'Pending',
    
    assigned_to VARCHAR(100),
    assigned_date TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    completed_at TIMESTAMP,
    
    expected_recovery_amount DECIMAL(15, 2),
    actual_recovery_amount DECIMAL(15, 2) NOT NULL DEFAULT 0,
    
    attempt_count INTEGER NOT NULL DEFAULT 0,
    notes TEXT,
    
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    
    CONSTRAINT valid_expected_amount CHECK (expected_recovery_amount IS NULL OR expected_recovery_amount >= 0),
    CONSTRAINT valid_actual_amount CHECK (actual_recovery_amount >= 0),
    CONSTRAINT valid_attempts CHECK (attempt_count >= 0)
);

CREATE INDEX idx_recovery_strategies_account ON recovery_strategies(account_id);
CREATE INDEX idx_recovery_strategies_status ON recovery_strategies(strategy_status);
CREATE INDEX idx_recovery_strategies_type ON recovery_strategies(strategy_type);

CREATE TRIGGER trigger_recovery_strategies_updated_at
    BEFORE UPDATE ON recovery_strategies
    FOR EACH ROW
    EXECUTE FUNCTION update_updated_at_column();

COMMENT ON TABLE recovery_strategies IS '[CORE MVP] Stores recovery strategies for delinquent loan accounts - implements Strategy Pattern';

-- Channel Type Enum
CREATE TYPE channel_type_enum AS ENUM (
    'Email',
    'SMS',
    'Phone',
    'Letter'
);

-- Message Status Enum
CREATE TYPE message_status_enum AS ENUM (
    'Pending',
    'Sent',
    'Delivered',
    'Failed',
    'Opened'
);

-- Communication Logs Table
CREATE TABLE IF NOT EXISTS communication_logs (
    communication_id SERIAL PRIMARY KEY,
    account_id INTEGER NOT NULL REFERENCES loan_accounts(account_id) ON DELETE CASCADE,
    borrower_id INTEGER NOT NULL REFERENCES borrowers(borrower_id) ON DELETE CASCADE,
    strategy_id INTEGER REFERENCES recovery_strategies(strategy_id) ON DELETE SET NULL,
    
    channel_type channel_type_enum NOT NULL,
    message_content TEXT NOT NULL,
    message_status message_status_enum NOT NULL DEFAULT 'Pending',
    
    sent_at TIMESTAMP,
    delivered_at TIMESTAMP,
    error_message TEXT,
    
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    
    CONSTRAINT valid_delivered CHECK (
        (message_status = 'Delivered' AND delivered_at IS NOT NULL) OR
        (message_status != 'Delivered')
    )
);

CREATE INDEX idx_communication_logs_account ON communication_logs(account_id);
CREATE INDEX idx_communication_logs_strategy ON communication_logs(strategy_id) WHERE strategy_id IS NOT NULL;
CREATE INDEX idx_communication_logs_status ON communication_logs(message_status);

CREATE TRIGGER trigger_communication_logs_updated_at
    BEFORE UPDATE ON communication_logs
    FOR EACH ROW
    EXECUTE FUNCTION update_updated_at_column();

COMMENT ON TABLE communication_logs IS '[ADVANCED FEATURE] Tracks all communications sent to borrowers via Email/SMS/Phone/Letter (simulated for MVP)';

-- Automation Type Enum
CREATE TYPE automation_type_enum AS ENUM (
    'AutoReminder',
    'RiskReassess',
    'StrategyEscalate',
    'PaymentFollowup'
);

-- Automation Status Enum
CREATE TYPE automation_status_enum AS ENUM (
    'Active',
    'Paused',
    'Completed',
    'Failed'
);

-- Borrower Segments
CREATE TABLE IF NOT EXISTS borrower_segments (
    segment_id SERIAL PRIMARY KEY,
    account_id INTEGER NOT NULL REFERENCES loan_accounts(account_id) ON DELETE CASCADE,
    borrower_id INTEGER NOT NULL REFERENCES borrowers(borrower_id) ON DELETE CASCADE,
    
    cluster_id INTEGER NOT NULL,
    cluster_distance DECIMAL(8, 4),
    segment_characteristics JSONB,
    
    algorithm_used VARCHAR(50) NOT NULL DEFAULT 'KMeans',
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_borrower_segments_account ON borrower_segments(account_id);
CREATE INDEX idx_borrower_segments_cluster ON borrower_segments(cluster_id);

CREATE TRIGGER trigger_borrower_segments_updated_at
    BEFORE UPDATE ON borrower_segments
    FOR EACH ROW
    EXECUTE FUNCTION update_updated_at_column();

COMMENT ON TABLE borrower_segments IS '[ADVANCED FEATURE - ML] K-Means clustering results for borrower segmentation - enables targeted recovery strategies';

-- Process Automation Table
CREATE TABLE IF NOT EXISTS process_automation (
    automation_id SERIAL PRIMARY KEY,
    account_id INTEGER NOT NULL REFERENCES loan_accounts(account_id) ON DELETE CASCADE,
    
    automation_type automation_type_enum NOT NULL,
    automation_status automation_status_enum NOT NULL DEFAULT 'Active',
    
    trigger_condition JSONB,
    execution_count INTEGER NOT NULL DEFAULT 0,
    last_execution TIMESTAMP,
    next_execution TIMESTAMP,
    
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_process_automation_account ON process_automation(account_id);
CREATE INDEX idx_process_automation_status ON process_automation(automation_status);
CREATE INDEX idx_process_automation_next ON process_automation(next_execution) 
    WHERE automation_status = 'Active' AND next_execution IS NOT NULL;

CREATE TRIGGER trigger_process_automation_updated_at
    BEFORE UPDATE ON process_automation
    FOR EACH ROW
    EXECUTE FUNCTION update_updated_at_column();

COMMENT ON TABLE process_automation IS '[ADVANCED FEATURE] Process automation rules per account - automates reminders, risk reassessments, strategy escalation';

-- Operation Type Enum
CREATE TYPE operation_type_enum AS ENUM (
    'INSERT',
    'UPDATE',
    'DELETE'
);

-- Audit Logs Table
CREATE TABLE IF NOT EXISTS audit_logs (
    log_id SERIAL PRIMARY KEY,
    table_name VARCHAR(50) NOT NULL,
    operation operation_type_enum NOT NULL,
    record_id VARCHAR(50),
    
    old_values JSONB,
    new_values JSONB,
    changed_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX idx_audit_logs_table ON audit_logs(table_name);
CREATE INDEX idx_audit_logs_operation ON audit_logs(operation);
CREATE INDEX idx_audit_logs_date ON audit_logs(changed_at DESC);
CREATE INDEX idx_audit_logs_record ON audit_logs(table_name, record_id);

COMMENT ON TABLE audit_logs IS '[ADVANCED FEATURE] Audit trail for data changes - tracks all INSERT/UPDATE/DELETE operations for compliance';

-- ML Model Table
CREATE TABLE IF NOT EXISTS ml_models (
    model_id SERIAL PRIMARY KEY,
    model_name VARCHAR(50) NOT NULL,
    model_type VARCHAR(50) NOT NULL, -- 'RandomForest' or 'KMeans'
    version VARCHAR(20) NOT NULL,
    
    hyperparameters JSONB,
    metrics JSONB, -- accuracy, precision, recall, f1_score, silhouette_score
    
    trained_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    is_active BOOLEAN DEFAULT false,
    
    CONSTRAINT unique_active_model UNIQUE (model_type, is_active) 
        WHERE is_active = true
);

CREATE INDEX idx_ml_models_type ON ml_models(model_type);
CREATE INDEX idx_ml_models_active ON ml_models(is_active) WHERE is_active = true;
CREATE INDEX idx_ml_models_trained ON ml_models(trained_at DESC);
CREATE INDEX idx_ml_models_type_version ON ml_models(model_type, version);

CREATE TRIGGER trigger_ml_models_updated_at
    BEFORE UPDATE ON ml_models
    FOR EACH ROW
    EXECUTE FUNCTION update_updated_at_column();

COMMENT ON TABLE ml_models IS '[OPTIONAL - ML] Model version control for RandomForest and K-Means - tracks hyperparameters, metrics, and active versions';
COMMENT ON COLUMN ml_models.model_id IS 'Primary key, auto-incrementing model ID';
COMMENT ON COLUMN ml_models.model_name IS 'Human-readable model name (e.g., "Risk_Predictor_v1", "Borrower_Segmenter_v2")';
COMMENT ON COLUMN ml_models.model_type IS 'Type of ML algorithm: "RandomForest" for risk assessment or "KMeans" for segmentation';
COMMENT ON COLUMN ml_models.version IS 'Model version string (e.g., "1.0.0", "2.1.3")';
COMMENT ON COLUMN ml_models.hyperparameters IS 'JSONB storing model hyperparameters (e.g., {"n_estimators": 100, "max_depth": 10, "k_clusters": 5})';
COMMENT ON COLUMN ml_models.metrics IS 'JSONB storing model performance metrics (e.g., {"accuracy": 0.89, "precision": 0.85, "recall": 0.92, "silhouette_score": 0.67})';
COMMENT ON COLUMN ml_models.trained_at IS 'Timestamp when model was trained';
COMMENT ON COLUMN ml_models.is_active IS 'Whether this model version is currently in production (only one active model per type)';