def get_calllog_count(harness):
    body = {"count": True}
    return harness.endpoint_request("calllog", "get", body)["body"]["count"]
