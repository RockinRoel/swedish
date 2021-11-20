# Swedish: solve crossword puzzles together

## Build and run

### Podman

#### Build

```sh
podman build -t swedish .
```

### Run

Create a `config` folder with`wt_config.xml`:
```xml
<server>
  <application-settings location="*">
    <max-request-size>10240</max-request-size>
    <web-sockets>true</web-sockets>
    <properties>
      <property name="connection_string">host=127.0.0.1 port=5432 dbname=swedish user=swedish password=mypassword</property>
    </properties>
    <log-config>* -info -debug</log-config>
    <trusted-proxy-config>
      <original-ip-header>X-Forwarded-For</original-ip-header>
      <trusted-proxies>
        <proxy>127.0.0.1/8</proxy>
        <proxy>::1/128</proxy>
      </trusted-proxies>
    </trusted-proxy-config>
  </application-settings>
</server>
```

```sh
podman pod create --name swedish-pod --publish 127.0.0.1:8002:8002
podman run --pod swedish-pod --name swedish-postgres -e POSTGRES_USER=swedish -e POSTGRES_PASSWORD=mypassword -d postgres
podman run --pod swedish-pod --name swedish-app -v ./config:/swedish/config:Z -d swedish
```
