// API Client for Smart Debt Recovery System
// Routes through Python proxy server to avoid CORS issues

const API_BASE_URL = 'http://localhost:3000';

class SDRSApi {
    constructor(baseUrl = API_BASE_URL) {
        this.baseUrl = baseUrl;
    }

    // Helper method for making requests
    async request(endpoint, options = {}) {
        const url = `${this.baseUrl}${endpoint}`;
        const config = {
            headers: {
                'Content-Type': 'application/json',
                ...options.headers,
            },
            ...options,
        };

        try {
            const response = await fetch(url, config);
            const data = await response.json();
            
            if (!response.ok) {
                throw new Error(data.message || `HTTP ${response.status}`);
            }
            
            return data;
        } catch (error) {
            console.error('API Request failed:', error);
            throw error;
        }
    }

    // Health & Info
    async getHealth() {
        return this.request('/health');
    }

    async getInfo() {
        return this.request('/');
    }

    // Borrower Service
    async getBorrowers() {
        return this.request('/api/borrowers');
    }

    async getBorrower(id) {
        return this.request(`/api/borrowers/${id}`);
    }

    async createBorrower(borrowerData) {
        return this.request('/api/borrowers', {
            method: 'POST',
            body: JSON.stringify(borrowerData),
        });
    }

    async updateBorrower(id, borrowerData) {
        return this.request(`/api/borrowers/${id}`, {
            method: 'PUT',
            body: JSON.stringify(borrowerData),
        });
    }

    async deleteBorrower(id) {
        return this.request(`/api/borrowers/${id}`, {
            method: 'DELETE',
        });
    }
    
    async updateBorrowerSegment(id, segment) {
        // GET borrower first, update segment, then PUT back
        const borrower = await this.request(`/api/borrowers/${id}`);
        const updated = { ...borrower.data, risk_segment: segment };
        return this.request(`/api/borrowers/${id}`, {
            method: 'PUT',
            body: JSON.stringify(updated),
        });
    }
    
    // Loan Accounts
    async getLoanAccounts() {
        return this.request('/api/loans/accounts');
    }
    
    async getLoanAccountsByBorrower(borrowerId) {
        return this.request(`/api/loans/borrower/${borrowerId}`);
    }
    
    async createLoan(loanData) {
        return this.request('/api/loans', {
            method: 'POST',
            body: JSON.stringify(loanData),
        });
    }
    
    // Payment History
    async getPaymentHistory(borrowerId) {
        return this.request(`/api/payments/borrower/${borrowerId}`);
    }

    // Risk Assessment Service
    async assessRisk(riskData) {
        return this.request('/api/risk/assess', {
            method: 'POST',
            body: JSON.stringify(riskData),
        });
    }
    
    async clusterBorrowers(clusterData) {
        return this.request('/api/risk/cluster', {
            method: 'POST',
            body: JSON.stringify(clusterData),
        });
    }

    // Recovery Strategy Service
    async getStrategies() {
        return this.request('/api/strategy/list');
    }

    async executeStrategy(strategyData) {
        return this.request('/api/strategy/execute', {
            method: 'POST',
            body: JSON.stringify(strategyData),
        });
    }

    // Communication Service
    async sendEmail(emailData) {
        return this.request('/api/communication/email', {
            method: 'POST',
            body: JSON.stringify(emailData),
        });
    }

    async sendSMS(smsData) {
        return this.request('/api/communication/sms', {
            method: 'POST',
            body: JSON.stringify(smsData),
        });
    }

    async getCommunicationHistory(borrowerId) {
        return this.request(`/api/communication/history/${borrowerId}`);
    }
}

// Utility functions
function formatCurrency(amount) {
    return new Intl.NumberFormat('vi-VN', {
        style: 'currency',
        currency: 'VND',
    }).format(amount);
}

function formatDate(dateString) {
    return new Date(dateString).toLocaleDateString('vi-VN', {
        year: 'numeric',
        month: 'long',
        day: 'numeric',
        hour: '2-digit',
        minute: '2-digit',
    });
}

function getRiskClass(riskLevel) {
    const level = riskLevel.toLowerCase();
    if (level.includes('low')) return 'risk-low';
    if (level.includes('medium')) return 'risk-medium';
    if (level.includes('high')) return 'risk-high';
    return '';
}

function getRiskBadge(riskLevel) {
    const level = riskLevel.toLowerCase();
    if (level.includes('low')) return 'success';
    if (level.includes('medium')) return 'warning';
    if (level.includes('high')) return 'danger';
    return 'secondary';
}

function showAlert(message, type = 'info') {
    const alertDiv = document.createElement('div');
    alertDiv.className = `alert alert-${type} alert-dismissible fade show`;
    alertDiv.innerHTML = `
        ${message}
        <button type="button" class="btn-close" data-bs-dismiss="alert"></button>
    `;
    
    const container = document.querySelector('.container');
    container.insertBefore(alertDiv, container.firstChild);
    
    setTimeout(() => {
        alertDiv.remove();
    }, 5000);
}

function showLoading(containerId) {
    const container = document.getElementById(containerId);
    container.innerHTML = `
        <div class="spinner-container">
            <div class="spinner-border text-primary" role="status">
                <span class="visually-hidden">Loading...</span>
            </div>
            <p class="mt-3">Loading data...</p>
        </div>
    `;
}

function hideLoading(containerId) {
    const container = document.getElementById(containerId);
    if (container.querySelector('.spinner-container')) {
        container.innerHTML = '';
    }
}

// Export for use in HTML pages
const api = new SDRSApi();
