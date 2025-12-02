<?php
require_once '../includes/config.php';

header('Content-Type: application/json');

if (!isLoggedIn()) {
    echo json_encode(['success' => false, 'message' => 'Unauthorized']);
    exit;
}

$userId = getCurrentUserId();
$isAdmin = isAdmin();
$isJanitor = isJanitor();
$filter = $_GET['filter'] ?? 'all';

// Base query
$sql = "
    SELECT 
        n.notification_id,
        n.notification_type,
        n.title,
        n.message,
        n.is_read,
        n.created_at,
        b.bin_code,
        b.location,
        CONCAT(j.first_name, ' ', j.last_name) AS janitor_name
    FROM notifications n
    LEFT JOIN bins b ON n.bin_id = b.bin_id
    LEFT JOIN janitors j ON n.janitor_id = j.janitor_id
    WHERE 1
";

// Admin = all
if ($isJanitor) {
    $sql .= " AND n.janitor_id = $userId ";
}

if ($filter !== 'all') {
    $safe = $conn->real_escape_string($filter);
    $sql .= " AND n.notification_type = '$safe' ";
}

$sql .= " ORDER BY n.created_at DESC LIMIT 50";

$result = $conn->query($sql);
$notifications = [];

while ($row = $result->fetch_assoc()) {
    $notifications[] = $row;
}

// Unread count logic
$unreadQuery = "SELECT COUNT(*) AS unread_count FROM notifications WHERE is_read = 0";

if ($isJanitor) {
    $unreadQuery .= " AND janitor_id = $userId";
}

$unreadRes = $conn->query($unreadQuery);
$unreadCount = $unreadRes->fetch_assoc()['unread_count'];

echo json_encode([
    'success' => true,
    'notifications' => $notifications,
    'unread_count' => $unreadCount
]);
?>
