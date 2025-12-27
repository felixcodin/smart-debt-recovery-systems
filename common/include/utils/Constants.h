#ifndef SDRS_CONSTANTS_H
#define SDRS_CONSTANTS_H

#include <string>
#include <cstdint>

namespace sdrs::constants
{

// ============================================================================
// APPLICATION INFO
// ============================================================================
namespace app
{
    inline constexpr const char* NAME = "Smart Debt Recovery System";
    inline constexpr const char* VERSION = "1.0.0";
    inline constexpr const char* ENVIRONMENT = "development"; // development, staging, production
}

// ============================================================================
// DATABASE CONFIGURATION
// ============================================================================
namespace database
{
    inline constexpr const char* DEFAULT_HOST = "localhost";
    inline constexpr int DEFAULT_PORT = 5432;
    inline constexpr const char* DEFAULT_NAME = "sdrs_db";
    inline constexpr const char* DEFAULT_USER = "sdrs_user";
    inline constexpr int CONNECTION_TIMEOUT_SEC = 30;
    inline constexpr int MAX_POOL_SIZE = 10;
    inline constexpr int MIN_POOL_SIZE = 2;
}

// ============================================================================
// SERVICE PORTS
// ============================================================================
namespace ports
{
    inline constexpr int API_GATEWAY = 8080;
    inline constexpr int BORROWER_SERVICE = 8081;
    inline constexpr int RISK_ASSESSMENT_SERVICE = 8082;
    inline constexpr int RECOVERY_STRATEGY_SERVICE = 8083;
    inline constexpr int COMMUNICATION_SERVICE = 8084;
}

enum class InactiveReason
{
    PaidOff,
    AccountClosed,
    Deceased,
    Fraud,
    LegalAction,
    None
};

enum class EmploymentStatus
{
    Employed,
    Unemployed,
    SelfEmployed,
    Student,
    Retired,
    Contract,
    PartTime,
    None
};

enum class AccountStatus
{
    Current,
    Delinquent,
    Partial,
    Default,
    PaidOff,
    ChargedOff,
    Settled
};

enum class PaymentStatus
{
    Pending,
    Completed,
    Failed,
    Cancelled
};

enum class PaymentMethod
{
    BankTransfer,
    Cash,
    Card,
    Other
};


// Error codes for communication operations
enum class CommunicationErrorCode
{
    SendFailed,          // Failed to send message (network/service error)
    InvalidRecipient,    // Invalid email/phone format
    ChannelUnavailable,  // Communication channel temporarily unavailable
    RateLimitExceeded,   // Too many messages sent in short period
    Unknown              // Unclassified communication error
};

// Communication channels available in the system
enum class ChannelType
{
    Email,      // Email channel (SMTP)
    SMS,        // SMS channel (SMS gateway)
    VoiceCall   // Voice call channel (maps to "Phone" in database)
};


// Error codes for database operations
enum class DatabaseErrorCode
{
    ConnectionFailed,       // Cannot connect to database
    QueryFailed,           // SQL query execution failed
    RecordNotFound,        // SELECT returned no results
    ConstraintViolation,   // Generic database constraint violated
    UniqueViolation,       // UNIQUE constraint violated (e.g., duplicate email)
    ForeignKeyViolation,   // Foreign key constraint violated
    Unknown                // Unclassified database error
};

// Error codes for validation failures
enum class ValidationErrorCode
{
    ValidationFailed,  // General validation error
    MissingField,      // Required field is missing
    InvalidFormat      // Field format is incorrect (e.g., email, phone)
};

enum class StrategyType
{
    AutomatedReminder,
    SettlementOffer,
    LegalAction
};

enum class StrategyStatus
{
    Pending,
    Active,
    Completed,
    Failed,
    Cancelled
};

enum class LegalStage
{
    NotStarted,
    NoticesSent, // legal notice sent
    CaseFiled, // Lawsuit field
    InCourt, // Proceeding in court
    JudgmentObtained, // Judgment issued
    Enforcing // Enforcement underway
};

enum class RiskLevel
{
    Low, // < 0.33
    Medium, // 0.33 - 0.67
    High // >= 0.67
};

// ============================================================================
// RISK ASSESSMENT THRESHOLDS
// ============================================================================
namespace risk
{
    // Risk score boundaries (0.0 to 1.0)
    inline constexpr double LOW_RISK_MAX = 0.33;
    inline constexpr double MEDIUM_RISK_MAX = 0.67;

    // Days past due thresholds
    inline constexpr int DPD_LOW_THRESHOLD = 30;      // 1-30 days: Low risk
    inline constexpr int DPD_MEDIUM_THRESHOLD = 60;   // 31-60 days: Medium risk
    inline constexpr int DPD_HIGH_THRESHOLD = 90;     // 61-90 days: High risk
    inline constexpr int DPD_DEFAULT_THRESHOLD = 180; // >180 days: Default

    // Random Forest hyperparameters
    inline constexpr int RF_NUM_TREES = 10;
    inline constexpr int RF_MAX_DEPTH = 7;
    inline constexpr int RF_MIN_SAMPLES_SPLIT = 5;
    inline constexpr int RF_LEAF_FEATURES_INDEX = -1;
    inline constexpr double RF_VARIANCE_EPSILON = 1e-7;

    // K-Means hyperparameters
    inline constexpr int KMEANS_NUM_CLUSTERS = 3;
    inline constexpr int KMEANS_MAX_ITERATIONS = 100;
    inline constexpr double KMEANS_TOLERANCE = 1e-4;
}

// ============================================================================
// RECOVERY STRATEGY CONSTANTS
// ============================================================================
namespace recovery
{
    // Maximum attempt counts   
    inline constexpr int MAX_REMINDER_ATTEMPTS = 5;
    inline constexpr int MAX_SETTLEMENT_ATTEMPTS = 3;
    inline constexpr int MAX_LEGAL_ATTEMPTS = 1;
    inline constexpr int INTERVAL_DAYS = 7;

    // Late fee rate (percentage of monthly payment)
    inline constexpr double LATE_FEE_RATE = 0.02;  // 2% late fee

    // Settlement offer percentages
    inline constexpr double SETTLEMENT_DISCOUNT_LOW_RISK = 0.05;    // 5% discount
    inline constexpr double SETTLEMENT_DISCOUNT_MEDIUM_RISK = 0.10; // 10% discount
    inline constexpr double SETTLEMENT_DISCOUNT_HIGH_RISK = 0.15;   // 15% discount
    inline constexpr double MAX_SETTLEMENT_DISCOUNT = 0.25;         // Max 25% discount

    // Strategy escalation days
    inline constexpr int ESCALATE_TO_SETTLEMENT_DAYS = 30;
    inline constexpr int ESCALATE_TO_LEGAL_DAYS = 90;
    
    // Legal notice deadline (days)
    inline constexpr int LEGAL_NOTICE_DEADLINE_DAYS = 30;
}

// ============================================================================
// COMMUNICATION CONSTANTS
// ============================================================================
namespace communication
{
    // Retry settings
    inline constexpr int MAX_SEND_RETRIES = 3;
    inline constexpr int RETRY_DELAY_SECONDS = 60;

    // Rate limits (per hour)
    inline constexpr int EMAIL_RATE_LIMIT = 100;
    inline constexpr int SMS_RATE_LIMIT = 50;
    inline constexpr int VOICE_CALL_RATE_LIMIT = 20;

    // Message length limits
    inline constexpr size_t SMS_MAX_LENGTH = 160;
    inline constexpr size_t EMAIL_SUBJECT_MAX_LENGTH = 100;
    inline constexpr size_t EMAIL_BODY_MAX_LENGTH = 10000;

    // Template IDs
    inline constexpr const char* TEMPLATE_REMINDER_FIRST = "REMIND_01";
    inline constexpr const char* TEMPLATE_REMINDER_SECOND = "REMIND_02";
    inline constexpr const char* TEMPLATE_REMINDER_FINAL = "REMIND_FINAL";
    inline constexpr const char* TEMPLATE_SETTLEMENT_OFFER = "SETTLE_01";
    inline constexpr const char* TEMPLATE_LEGAL_NOTICE = "LEGAL_01";
}

// ============================================================================
// VALIDATION CONSTANTS
// ============================================================================
namespace validation
{
    // String length limits
    inline constexpr size_t NAME_MIN_LENGTH = 1;
    inline constexpr size_t NAME_MAX_LENGTH = 50;
    inline constexpr size_t EMAIL_MAX_LENGTH = 100;
    inline constexpr size_t PHONE_MIN_LENGTH = 10;
    inline constexpr size_t PHONE_MAX_LENGTH = 11;
    inline constexpr size_t ADDRESS_MAX_LENGTH = 500;

    // Financial limits
    inline constexpr double MIN_LOAN_AMOUNT = 100.0;
    inline constexpr double MAX_LOAN_AMOUNT = 1000000000.0;  // 1 billion
    inline constexpr double MIN_INTEREST_RATE = 0.0;
    inline constexpr double MAX_INTEREST_RATE = 1.0;         // 100%
    inline constexpr double MIN_MONTHLY_INCOME = 0.0;
    inline constexpr double MAX_DISCOUNT_RATE = 1.0;

    // Regex patterns
    inline constexpr const char* EMAIL_PATTERN = R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)";
    inline constexpr const char* PHONE_PATTERN = R"(^[0-9]{10,11}$)";
}

// ============================================================================
// MONEY & CURRENCY
// ============================================================================
namespace currency
{
    inline constexpr const char* DEFAULT_CURRENCY = "VND";
    inline constexpr double EPSILON = 0.01;  // For floating-point comparison
    inline constexpr int DECIMAL_PLACES = 2;

    // Exchange rates (to VND) - for reference only
    inline constexpr double USD_TO_VND = 26.312;
}

enum class MoneyType
{
    VND,
    USD
};

// ============================================================================
// TIME CONSTANTS
// ============================================================================
namespace time
{
    inline constexpr int SECONDS_PER_MINUTE = 60;
    inline constexpr int MINUTES_PER_HOUR = 60;
    inline constexpr int HOURS_PER_DAY = 24;
    inline constexpr int DAYS_PER_WEEK = 7;
    inline constexpr int DAYS_PER_MONTH = 30;  // Average
    inline constexpr int DAYS_PER_YEAR = 365;

    // Business hours (24h format)
    inline constexpr int BUSINESS_HOUR_START = 8;
    inline constexpr int BUSINESS_HOUR_END = 17;

    // Timeouts (in seconds)
    inline constexpr int HTTP_REQUEST_TIMEOUT = 30;
    inline constexpr int DATABASE_QUERY_TIMEOUT = 60;
    inline constexpr int SERVICE_HEALTH_CHECK_INTERVAL = 30;
}

// ============================================================================
// LOGGING CONSTANTS
// ============================================================================
namespace logging
{
    inline constexpr const char* DEFAULT_LOG_FILE = "sdrs.log";
    inline constexpr const char* LOG_DATE_FORMAT = "%Y-%m-%d %H:%M:%S";
    inline constexpr size_t MAX_LOG_FILE_SIZE_MB = 100;
    inline constexpr int MAX_LOG_FILES = 5;

    // Log levels
    inline constexpr int LEVEL_TRACE = 0;
    inline constexpr int LEVEL_DEBUG = 1;
    inline constexpr int LEVEL_INFO = 2;
    inline constexpr int LEVEL_WARNING = 3;
    inline constexpr int LEVEL_ERROR = 4;
    inline constexpr int LEVEL_CRITICAL = 5;
}

// ============================================================================
// API CONSTANTS
// ============================================================================
namespace api
{
    inline constexpr const char* API_VERSION = "v1";
    inline constexpr const char* BASE_PATH = "/api/v1";

    // Pagination defaults
    inline constexpr int DEFAULT_PAGE_SIZE = 20;
    inline constexpr int MAX_PAGE_SIZE = 100;

    // Rate limiting
    inline constexpr int RATE_LIMIT_REQUESTS_PER_MINUTE = 60;
    inline constexpr int RATE_LIMIT_WINDOW_SECONDS = 60;
}


} // namespace sdrs::constants

#endif // SDRS_CONSTANTS_H
