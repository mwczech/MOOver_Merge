#!/bin/bash

# MELKENS System Validation Suite
# Complete automated testing and validation script

set -e  # Exit on any error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
NC='\033[0m' # No Color

# Configuration
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
REPORT_DIR="validation_reports"
REPORT_FILE="$REPORT_DIR/validation_report_$TIMESTAMP.md"
LOG_FILE="$REPORT_DIR/validation_log_$TIMESTAMP.log"
API_BASE_URL="http://localhost:3001"
FRONTEND_URL="http://localhost:3000"

# Test results tracking
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0
WARNINGS=0

# Create report directory
mkdir -p "$REPORT_DIR"

# Initialize report
cat > "$REPORT_FILE" << EOF
# MELKENS System Validation Report

**Generated:** $(date)  
**Version:** 1.0.0  
**Environment:** $(uname -s) $(uname -r)

## Executive Summary

EOF

# Logging function
log() {
    echo -e "${2:-$NC}$1$NC" | tee -a "$LOG_FILE"
}

# Test result tracking functions
test_start() {
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    log "🧪 Starting test: $1" "$BLUE"
}

test_pass() {
    PASSED_TESTS=$((PASSED_TESTS + 1))
    log "✅ PASS: $1" "$GREEN"
    echo "- ✅ **PASS:** $1" >> "$REPORT_FILE"
}

test_fail() {
    FAILED_TESTS=$((FAILED_TESTS + 1))
    log "❌ FAIL: $1" "$RED"
    echo "- ❌ **FAIL:** $1" >> "$REPORT_FILE"
}

test_warn() {
    WARNINGS=$((WARNINGS + 1))
    log "⚠️  WARN: $1" "$YELLOW"
    echo "- ⚠️  **WARN:** $1" >> "$REPORT_FILE"
}

# Check if service is running
check_service() {
    local service_name=$1
    local url=$2
    local expected_status=${3:-200}
    
    test_start "Service availability: $service_name"
    
    if curl -s -o /dev/null -w "%{http_code}" "$url" | grep -q "$expected_status"; then
        test_pass "$service_name is running and responding"
        return 0
    else
        test_fail "$service_name is not responding or returned unexpected status"
        return 1
    fi
}

# API endpoint test
test_api_endpoint() {
    local method=$1
    local endpoint=$2
    local expected_status=${3:-200}
    local payload=$4
    
    test_start "API Endpoint: $method $endpoint"
    
    local cmd="curl -s -o /dev/null -w '%{http_code}' -X $method"
    
    if [ -n "$payload" ]; then
        cmd="$cmd -H 'Content-Type: application/json' -d '$payload'"
    fi
    
    cmd="$cmd $API_BASE_URL$endpoint"
    
    local status_code
    status_code=$(eval "$cmd")
    
    if [ "$status_code" = "$expected_status" ]; then
        test_pass "$method $endpoint returned $status_code"
        return 0
    else
        test_fail "$method $endpoint returned $status_code, expected $expected_status"
        return 1
    fi
}

# Performance test
performance_test() {
    local endpoint=$1
    local max_time=${2:-100}
    
    test_start "Performance: $endpoint response time"
    
    local response_time
    response_time=$(curl -s -o /dev/null -w "%{time_total}" "$API_BASE_URL$endpoint" | awk '{print $1*1000}')
    response_time=${response_time%.*}  # Remove decimal part
    
    if [ "$response_time" -lt "$max_time" ]; then
        test_pass "$endpoint responds in ${response_time}ms (< ${max_time}ms)"
    elif [ "$response_time" -lt $((max_time * 2)) ]; then
        test_warn "$endpoint responds in ${response_time}ms (> ${max_time}ms but acceptable)"
    else
        test_fail "$endpoint responds in ${response_time}ms (>> ${max_time}ms)"
    fi
}

# Start validation
log "🚀 Starting MELKENS System Validation Suite" "$PURPLE"
log "📊 Report will be generated at: $REPORT_FILE" "$BLUE"

# Add test results section to report
cat >> "$REPORT_FILE" << EOF
## Test Results

### Service Availability Tests
EOF

# Phase 1: Service Availability
log "\n📡 Phase 1: Service Availability Tests" "$YELLOW"
check_service "Backend API" "$API_BASE_URL/api/health"
check_service "Frontend Application" "$FRONTEND_URL"

# Phase 2: API Functionality Tests
log "\n🔧 Phase 2: API Functionality Tests" "$YELLOW"
cat >> "$REPORT_FILE" << EOF

### API Functionality Tests
EOF

# Test core API endpoints
test_api_endpoint "GET" "/api/health"
test_api_endpoint "GET" "/api/robot/status"
test_api_endpoint "GET" "/api/routes"
test_api_endpoint "GET" "/api/logs"
test_api_endpoint "GET" "/api/settings"

# Phase 3: Performance Tests
log "\n⚡ Phase 3: Performance Tests" "$YELLOW"
cat >> "$REPORT_FILE" << EOF

### Performance Tests
EOF

performance_test "/api/health" 50
performance_test "/api/robot/status" 100
performance_test "/api/routes" 200
performance_test "/api/logs" 300

# Phase 4: Route Management Tests
log "\n🗺️  Phase 4: Route Management Tests" "$YELLOW"
cat >> "$REPORT_FILE" << EOF

### Route Management Tests
EOF

# Create test route
TEST_ROUTE='{
  "name": "Test Route",
  "description": "Automated test route",
  "steps": [
    {"id": 0, "operation": "NORM", "position": {"x": 0, "y": 0}, "speed": 0.3},
    {"id": 1, "operation": "NORM", "position": {"x": 100, "y": 0}, "speed": 0.3},
    {"id": 2, "operation": "STOP", "position": {"x": 100, "y": 0}, "speed": 0}
  ]
}'

test_api_endpoint "POST" "/api/routes" 201 "$TEST_ROUTE"
test_api_endpoint "GET" "/api/routes"

# Phase 5: Event Injection Tests
log "\n💉 Phase 5: Event Injection Tests" "$YELLOW"
cat >> "$REPORT_FILE" << EOF

### Event Injection Tests
EOF

# Test event injection endpoints
test_api_endpoint "GET" "/api/events/active"
test_api_endpoint "POST" "/api/events/inject-sensor" 200 '{"sensorType": "battery", "value": {"voltage": 11.5}, "duration": 1000}'
test_api_endpoint "DELETE" "/api/events/clear" 200

# Phase 6: Hardware Integration Tests (if available)
log "\n🔌 Phase 6: Hardware Integration Tests" "$YELLOW"
cat >> "$REPORT_FILE" << EOF

### Hardware Integration Tests
EOF

# Check if hardware bridge is available
if curl -s "$API_BASE_URL/api/hardware/status" | grep -q "connected"; then
    test_pass "Hardware bridge service is available"
    test_api_endpoint "GET" "/api/hardware/status"
    test_api_endpoint "GET" "/api/hardware/messages"
else
    test_warn "Hardware bridge not connected (expected in development)"
fi

# Phase 7: Validation Engine Tests
log "\n✅ Phase 7: Validation Engine Tests" "$YELLOW"
cat >> "$REPORT_FILE" << EOF

### Validation Engine Tests
EOF

test_api_endpoint "GET" "/api/validation/results"
test_api_endpoint "POST" "/api/validation/validate" 200 '{
  "routeId": "test_route",
  "logs": [],
  "finalState": {"position": {"x": 100, "y": 0}, "orientation": 0},
  "metadata": {"testRun": "automated_validation"}
}'

# Phase 8: Fault Injector CLI Tests
log "\n🚨 Phase 8: Fault Injector Tests" "$YELLOW"
cat >> "$REPORT_FILE" << EOF

### Fault Injector Tests
EOF

# Check if fault injector is available
if command -v npx >/dev/null 2>&1; then
    test_start "Fault injector CLI availability"
    if npx fault-injector --help >/dev/null 2>&1; then
        test_pass "Fault injector CLI is available and functional"
        
        # Test scenario listing
        test_start "Fault injector scenario listing"
        if npx fault-injector list >/dev/null 2>&1; then
            test_pass "Fault injector can list scenarios"
        else
            test_fail "Fault injector cannot list scenarios"
        fi
    else
        test_fail "Fault injector CLI is not functional"
    fi
else
    test_warn "npx not available, skipping fault injector CLI tests"
fi

# Phase 9: Load Testing (Basic)
log "\n📈 Phase 9: Basic Load Testing" "$YELLOW"
cat >> "$REPORT_FILE" << EOF

### Load Testing
EOF

test_start "Concurrent API requests"
# Simple load test with 10 concurrent requests
if command -v curl >/dev/null 2>&1; then
    success_count=0
    for i in {1..10}; do
        if curl -s "$API_BASE_URL/api/health" >/dev/null; then
            success_count=$((success_count + 1))
        fi &
    done
    wait
    
    if [ "$success_count" -eq 10 ]; then
        test_pass "All 10 concurrent requests succeeded"
    elif [ "$success_count" -ge 8 ]; then
        test_warn "$success_count/10 concurrent requests succeeded"
    else
        test_fail "Only $success_count/10 concurrent requests succeeded"
    fi
else
    test_warn "curl not available for load testing"
fi

# Phase 10: Security Tests (Basic)
log "\n🔒 Phase 10: Basic Security Tests" "$YELLOW"
cat >> "$REPORT_FILE" << EOF

### Security Tests
EOF

# Test CORS headers
test_start "CORS headers present"
if curl -s -I "$API_BASE_URL/api/health" | grep -i "access-control" >/dev/null; then
    test_pass "CORS headers are present"
else
    test_warn "CORS headers not detected"
fi

# Test for sensitive information exposure
test_start "No sensitive information in error responses"
error_response=$(curl -s "$API_BASE_URL/api/nonexistent" 2>/dev/null || true)
if echo "$error_response" | grep -i -E "(password|secret|key|token)" >/dev/null; then
    test_fail "Potential sensitive information exposure in error responses"
else
    test_pass "No sensitive information detected in error responses"
fi

# Generate final report
log "\n📊 Generating Final Report..." "$PURPLE"

# Calculate success rate
if [ "$TOTAL_TESTS" -gt 0 ]; then
    SUCCESS_RATE=$((PASSED_TESTS * 100 / TOTAL_TESTS))
else
    SUCCESS_RATE=0
fi

# Determine overall status
if [ "$FAILED_TESTS" -eq 0 ] && [ "$WARNINGS" -eq 0 ]; then
    OVERALL_STATUS="✅ PASS"
    STATUS_COLOR="$GREEN"
elif [ "$FAILED_TESTS" -eq 0 ]; then
    OVERALL_STATUS="⚠️ PASS WITH WARNINGS"
    STATUS_COLOR="$YELLOW"
else
    OVERALL_STATUS="❌ FAIL"
    STATUS_COLOR="$RED"
fi

# Add summary to report
cat >> "$REPORT_FILE" << EOF

## Summary

**Overall Status:** $OVERALL_STATUS  
**Success Rate:** $SUCCESS_RATE%  
**Total Tests:** $TOTAL_TESTS  
**Passed:** $PASSED_TESTS  
**Failed:** $FAILED_TESTS  
**Warnings:** $WARNINGS  

## Recommendations

EOF

if [ "$FAILED_TESTS" -eq 0 ] && [ "$WARNINGS" -eq 0 ]; then
    cat >> "$REPORT_FILE" << EOF
🎉 **System is ready for production deployment!**

All tests passed successfully. The system demonstrates:
- Excellent performance characteristics
- Robust API functionality  
- Reliable service availability
- Proper security measures

**Next Steps:**
1. Deploy to production environment
2. Configure monitoring and alerting
3. Perform user acceptance testing
4. Update documentation as needed
EOF
elif [ "$FAILED_TESTS" -eq 0 ]; then
    cat >> "$REPORT_FILE" << EOF
⚠️ **System is ready for deployment with minor considerations**

All critical tests passed with some warnings noted. Consider:
- Review warning items for potential improvements
- Monitor performance metrics closely after deployment
- Plan to address warnings in next iteration

**Next Steps:**
1. Review and assess warning items
2. Deploy to staging environment first
3. Monitor performance metrics
4. Address warnings in next release
EOF
else
    cat >> "$REPORT_FILE" << EOF
❌ **System requires attention before deployment**

Critical issues found that should be resolved:
- $FAILED_TESTS test(s) failed
- Review failed tests and address underlying issues
- Re-run validation after fixes

**Next Steps:**
1. **PRIORITY:** Fix all failed tests
2. Re-run validation suite
3. Consider additional testing for affected areas
4. Deploy only after all tests pass
EOF
fi

# Add technical details
cat >> "$REPORT_FILE" << EOF

## Technical Details

**Test Environment:**
- OS: $(uname -s) $(uname -r)
- Date: $(date)
- Backend URL: $API_BASE_URL
- Frontend URL: $FRONTEND_URL

**Files Generated:**
- Report: $REPORT_FILE
- Logs: $LOG_FILE

---

*This report was generated automatically by the MELKENS validation suite.*
EOF

# Display final results
log "\n🎯 VALIDATION COMPLETE" "$PURPLE"
log "Overall Status: $OVERALL_STATUS" "$STATUS_COLOR"
log "Success Rate: $SUCCESS_RATE% ($PASSED_TESTS/$TOTAL_TESTS)" "$BLUE"
log "📄 Full report: $REPORT_FILE" "$BLUE"
log "📋 Detailed logs: $LOG_FILE" "$BLUE"

# Exit with appropriate code
if [ "$FAILED_TESTS" -eq 0 ]; then
    exit 0
else
    exit 1
fi