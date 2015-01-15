<?php

/**
 * HandlerSocketi
 */
Class HandlerSocket
{

    /**
     * Configuration
     *
     * @var array
     */
    protected $_config = array();

    /**
     * Database name
     * @var string
     */
    protected $_db;

    /**
     * Table
     * @var string
     */
    protected $_table;

    /**
     * only read port
     * @var integer
     */
    protected $_port = 9998;

    /**
     * write and read port
     * @var integer
     */
    protected $_portWr = 9999;

    /**
     * Database read connection
     *
     * @var object|resource|null
     */
    protected $_connection = null;

    /**
     * Database write and read  connection
     *
     * @var object|resource|null
     */
    protected $_writeConnection = null;

    /**
     *  Last error log
     *
     * @var string
     */
    protected $_errorLog;

    /**
     * Constructor.
     *
     * $config is an array of key/value pairs
     * containing configuration options.
     *
     * host     => (string) What host to connect to, defaults to localhost
     * port     => (int) The port of the database read
     * port_wr  => (string) What host to connect to, defaults to localhost   => (int) The port of the database write, update, delete
     * database => (string) The name of the database to user
     * table    => (string) The table of the database table
     * options  => (array) set timeout, default 5s
     *
     * @param array $config
     */

    /**
     * Constructor
     *
     * @param array $config
     */
    public function __construct(array $config = array())
    {
        $this->_config = $config + array('host' => '127.0.0.1', 'port' => 9998, 'port_wr' => 9999, 'options' => array('timeout' => 5));

        $this->_port = $this->_config['port'];
        $this->_portWr = $this->_config['port_wr'];

        if (isset($this->_config['database'])) {
            $this->_db = $this->_config['database'];
        }

        if (isset($this->_config['table'])) {
            $this->_table = $this->_config['table'];
        }

        return $this->connection();
    }

    /**
     * Get HandlerSocketi connection
     *
     * @return HandlerSocketi
     */
    public function connection()
    {
        if (NULL === $this->_connection) {
            $this->_connection = new HandlerSocketi($this->_config['host'], $this->_port, $this->_config['options']);
        }
        if (isset($this->_config['auth'])) {
            $this->_connection->auth($this->_config['auth']['user'], $this->_config['auth']['password']);
        }
        return $this->_connection;
    }

    /**
     * Get HandlerSocketi write connection
     *
     * @return HandlerSocketi
     */
    public function writeConnection()
    {
        if (NULL === $this->_writeConnection) {
            $this->_writeConnection = new HandlerSocketi($this->_config['host'], $this->_portWr, $this->_config['options']);
        }
        if (isset($this->_config['auth'])) {
            $this->_writeConnection->auth($this->_config['auth']['user'], $this->_config['auth']['password']);
        }

        return $this->_writeConnection;
    }

    /**
     * Select Database
     *
     * @param string $db
     * @return database name
     */
    public function db($db = null)
    {
        if ($db) {
            return $this->_db->$db;
        }

        return $this->_db;
    }

    /**
     * Select table
     *
     * @param string $table
     * @return table name
     */
    public function talbe($table = NULL)
    {
        if ($table) {
            return $this->_table->$table;
        }

        return $this->_table;
    }

    /**
     * select and return query result array.
     *
     * Pass the query and options as array objects (this is more convenient than the standard HandlerSocketi API especially when caching)
     *
     * $query may contain:
     *   op - string comparison operator, supported '=', '<', '<=', '>', '>='
     *   criteria - string | array comparison values
     *   limit - int limit number
     *   skip - int skip number
     *
     * $options may contain:
     *   dbname - string database name
     *   table - string select table name
     *   fields - string | array the fields to retrieve
     *   index - string index name.
     *
     * @param array $query
     * @param array $options
     * @return false|array
     * */
    public function select(array $query = array(), array $options = array())
    {
        $query += array('op' => '=', 'criteria' => array(), 'limit' => 1, 'skip' => 0,);
        $options += array('dbname' => '', 'table' => '', 'fields' => '', 'index' => '');

        $dbname = $options['dbname'] ? : $this->_db;
        $table = $options['table'] ? : $this->_table;
        $index = $options['index'] ? : 'PRIMARY';
        $hsi = $this->connection()->open_index($dbname, $table, $options['fields'], ['index' => $index]);
        $result = $hsi->find([$query['op'] => $query['criteria']], ['limit' => $query['limit'], 'offset' => $query['skip']]);
        if (!$result) {
            $this->_errorLog = $hsi->getError();
        }
        return $result;
    }

    /**
     * selectMulti and return query result array.
     *
     * Pass the query and options as array objects (this is more convenient than the standard HandlerSocketi API especially when caching)
     *
     * $query may multi select parameter
     *
     * $options may contain:
     *   dbname - string database name
     *   table - string select table name
     *   fields - string | array the fields to retrieve
     *   index - string index name.
     *
     * @param array $querys
     * @param array $options
     * @return false|array
     * */
    public function selectMulti($querys = array(), $options = array())
    {
        $options += array('dbname' => '', 'table' => '', 'fields' => '', 'index' => '');

        $dbname = $options['dbname'] ? : $this->_db;
        $table = $options['table'] ? : $this->_table;
        $index = $options['index'] ? : 'PRIMARY';
        $hsi = $this->connection()->open_index($dbname, $table, $options['fields'], ['index' => $index]);

        foreach ($querys as $key => $value) {
            $query[$key] = ['find', [$value['op'] => $value['criteria']], ['limit' => $value['limit'], 'offset' => $value['skip']]];
        }
        $result = $hsi->multi($query);
        if (!$result) {
            $this->_errorLog = $hsi->getError();
        }
        return $result;
    }

    /**
     * Insert data
     *
     *  $data is insert fields array values
     *
     * $options may contain:
     *   dbname - database name
     *   table - select table name
     *
     * @param array $data
     * @param array $options
     * @return false|int
     * */
    public function insert($data, $options = array())
    {
        $options += array('dbname' => '', 'table' => '');
        $dbname = $options['dbname'] ? : $this->_db;
        $table = $options['table'] ? : $this->_table;
        $hsi = $this->writeConnection()->open_index($dbname, $table, implode(',', array_keys($data)));
        $resutl = $hsi->insert($data);
        if (!$resutl) {
            $this->_errorLog = $hsi->getError();
        }
        return $resutl;
    }

    /**
     * Update data
     *
     * $query may contain:
     *   op - string comparison operator, supported '=', '<', '<=', '>', '>='
     *   criteria - string | array comparison values
     *   limit - int limit number
     *   skip - int skip number
     *
     * $options may contain:
     *   dbname - string database name
     *   table - string select table name
     *   index - string index name.
     *
     * @param array $query
     * @param array $data
     * @param array $options
     * @return false|int
     */
    public function update($query, $data, $options = array())
    {
        $query += array('op' => '=', 'criteria' => array(), 'limit' => 1, 'skip' => 0);
        $options += array('dbname' => '', 'table' => '', 'index' => '');

        $dbname = $options['dbname'] ? : $this->_db;
        $table = $options['table'] ? : $this->_table;
        $index = $options['index'] ? : 'PRIMARY';

        $hsi = $this->writeConnection()->open_index($dbname, $table, implode(',', array_keys($data)), ['index' => $index]);
        $result = $hsi->update([$query['op'] => $query['criteria']], array_values($data), ['limit' => $query['limit'], 'offset' => $query['skip']]);
        if (!$result) {
            $this->_errorLog = $hsi->getError();
        }
        return $result;
    }

    /**
     * delete data
     * $query may contain:
     *   op - string comparison operator, supported '=', '<', '<=', '>', '>='
     *   criteria - string | array comparison values
     *   limit - int limit number
     *   skip - int skip number
     *
     * $options may contain:
     *   dbname - string database name
     *   table - string select table name
     *   fields - string | array the fields to retrieve
     *   index - string index name.
     *
     * @param array $query
     * @param array $options
     * @return false|int
     */
    public function delete($query, $options = array())
    {
        $query += array('op' => '=', 'criteria' => array(), 'limit' => 1, 'skip' => 0,);
        $options += array('dbname' => '', 'table' => '', 'fields' => '', 'index' => '');

        $dbname = $options['dbname'] ? : $this->_db;
        $table = $options['table'] ? : $this->_table;
        $index = $options['index'] ? : 'PRIMARY';

        $hsi = $this->writeConnection()->open_index($dbname, $table, $options['fields'], ['index' => $index]);
        $result = $hsi->remove([$query['op'] => $query['criteria']], ['limit' => $query['limit'], 'offset' => $query['skip']]);
        if (!$result) {
            $this->_errorLog = $hsi->getError();
        }
        return $result;
    }

    /**
     * Last error
     *
     * @return array
     */
    public function error($log = NULL)
    {
        if (NULL == $log) {
            return $this->_errorLog;
        }
        $this->_errorLog = $log;
    }

}
